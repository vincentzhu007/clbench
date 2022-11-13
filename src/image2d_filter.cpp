//
// Created by zgd on 11/13/22.
//

#include <iostream>
#include <string>

#include "FreeImage.h"
#include "utils.h"

#if __APPLE__
#include <OpenCL/opencl.hpp>
#else
#include <CL/cl2.hpp>
#endif

using namespace std;

void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message){
  printf("\n*** ");
  if(fif != FIF_UNKNOWN) {
    if (FreeImage_GetFormatFromFIF(fif))
      printf("%s Format\n", FreeImage_GetFormatFromFIF(fif));
  }
  printf(message);
  printf(" ***\n");
}

char *LoadImage(const std::string &file_name, size_t &width, size_t &height) {
  FREE_IMAGE_FORMAT format = FreeImage_GetFileType(file_name.c_str(), 0);
  FIBITMAP *image = FreeImage_Load(format, file_name.c_str());

  FIBITMAP *temp = image;
  image = FreeImage_ConvertTo32Bits(image);
  FreeImage_Unload(temp);

  width = FreeImage_GetWidth(image);
  height = FreeImage_GetHeight(image);

  char *buffer = new char[height * width * 4];
  memcpy(buffer, FreeImage_GetBits(image), height * width * 4);
  FreeImage_Unload(image);

  return buffer;
}

bool SaveImage(const std::string &file_name, void *buffer, size_t width, size_t height) {
  FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(file_name.c_str());
  FIBITMAP *image = FreeImage_ConvertFromRawBits((BYTE *)buffer, width, height, width * 4, 32,
                                                 0xFF000000, 0x00FF0000, 0x0000FF00, 0);
  FIBITMAP *image_24bits = FreeImage_ConvertTo24Bits(image);
  FreeImage_Unload(image);
  return FreeImage_Save(format, image_24bits, file_name.c_str()) == TRUE;
}

int main() {
  FreeImage_SetOutputMessage(FreeImageErrorHandler);

  // 预备工作1、定义kernel源码
  std::string kernel_source = R"(
    __constant sampler_t smp_zero = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

    __constant float gaussian_weights[9] = {1.0f, 2.0f, 1.0f,
                                            2.0f, 4.0f, 2.0f,
                                            1.0f, 1.0f, 1.0f};

    __kernel void test_image(__read_only image2d_t src_data, __write_only image2d_t dst_data) {
      int2 coord = {get_global_id(0), get_global_id(1)};
//      if (coord.x == 0 && coord.y == 0) {
//        printf("enter in kernel\n");
//      }
      int2 start_coord = {coord.x - 1, coord.y - 1};
      int2 end_coord = {coord.x + 1, coord.y + 1};

      int weight_idx = 0;
      float4 out_color = (float4){0.0f, 0.0f, 0.0f, 0.0f};
      for (int x = start_coord.x; x <= end_coord.x; x++) {
        for (int y = start_coord.y; y <= end_coord.y; y++) {
          float4 in_color = read_imagef(src_data, smp_zero, (int2)(x, y));
          out_color += in_color * (gaussian_weights[weight_idx] / 9.0f);
          weight_idx++;
        }
      }
      write_imagef(dst_data, coord, out_color);
    }
  )";

  // 预备工作2、读取图片到内存
  auto t_a = Utils::GetNowUs();
  size_t width = 0;
  size_t height = 0;
  const std::string image_file = "Lenna.jpg";
//  const std::string image_file = "test.jpg";
  auto in_buffer = LoadImage(image_file, width, height);
  auto t_b = Utils::GetNowUs();
  std::cout << "time(LoadImage): " << (t_b - t_a) << " us" << std::endl;

  try {
    // 1、获取平台和设备，创建上下文
    auto t_start = Utils::GetNowUs();
    std::vector<cl::Platform> platformList;
    cl::Platform::get(&platformList);
    auto t_1 = Utils::GetNowUs();
    std::cout << "time(Platform): " << (t_1 - t_start) << " us" << std::endl;

    cl_context_properties context_properties[] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)(platformList[0])(),
        0
    };
    cl::Context context(CL_DEVICE_TYPE_GPU, context_properties);
    auto t_2 = Utils::GetNowUs();
    std::cout << "time(Context): " << (t_2 - t_1) << " us" << std::endl;

    std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
    auto t_3 = Utils::GetNowUs();
    std::cout << "time(Device): " << (t_3 - t_2) << " us" << std::endl;

    // 2、构建程序
    cl::Program::Sources sources(1, std::make_pair(kernel_source.c_str(), 0));
    auto t_4 = Utils::GetNowUs();
    std::cout << "time(Sources): " << (t_4 - t_3) << " us" << std::endl;

    cl::Program program(context, sources);
    auto t_5 = Utils::GetNowUs();
    std::cout << "time(Program): " << (t_5 - t_4) << " us" << std::endl;

    try {
      program.build(devices);
    } catch (cl::Error &e) {
      if(program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(devices[0]) == CL_BUILD_ERROR) {
        auto build_log = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]);
        std::cout << "build opencl error: " << build_log << endl;
      }
    }
    auto t_6 = Utils::GetNowUs();
    std::cout << "time(build): " << (t_6 - t_5) << " us" << std::endl;

    // 3、创建cl内存对象
    cl_int err_num = CL_SUCCESS;
    cl::ImageFormat image_format;
    image_format.image_channel_order = CL_RGBA;
    image_format.image_channel_data_type = CL_UNORM_INT8;
    cl::Image2D in_image(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, // Create input image
                         image_format, width, height, 0, in_buffer, &err_num);
    if (err_num != CL_SUCCESS) {
      std::cerr << "failed to create input image, err_num: " << err_num << std::endl;
      return -1;
    }
    auto t_7 = Utils::GetNowUs();
    std::cout << "time(in_image): " << (t_7 - t_6) << " us" << std::endl;


    cl::Image2D out_image(context, CL_MEM_WRITE_ONLY, // Create output image
                         image_format, width, height, 0, nullptr, &err_num);
    if (err_num != CL_SUCCESS) {
      std::cerr << "failed to create output image, err_num: " << err_num << std::endl;
      return -1;
    }
    auto t_8 = Utils::GetNowUs();
    std::cout << "time(out_image): " << (t_8 - t_7) << " us" << std::endl;

    // 4、运行kernel
    cl::CommandQueue queue(context, devices[0], 0);
    auto t_9 = Utils::GetNowUs();
    std::cout << "time(CommandQueue): " << (t_9 - t_8) << " us" << std::endl;

    cl::Kernel kernel(program, "test_image");
    kernel.setArg(0, in_image);
    kernel.setArg(1, out_image);

    auto t_10 = Utils::GetNowUs();
    std::cout << "time(Kernel): " << (t_10 - t_9) << " us" << std::endl;

    cl::NDRange globalWorkSize(width, height);
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalWorkSize, cl::NullRange);
    queue.finish();
    auto t_11 = Utils::GetNowUs();
    std::cout << "time(enqueueNDRangeKernel): " << (t_11 - t_10) << " us" << std::endl;

    // 5、读取结果
//    char *out_buffer = new char[width * height * 4];
//    queue.enqueueReadImage(out_image, CL_TRUE, {0, 0, 0}, {width, height, 1}, 0, 0, out_buffer);
    void *out_buffer = queue.enqueueMapImage(out_image, CL_TRUE, CL_MAP_READ, {0, 0, 0}, {width, height, 1},
                                             nullptr, nullptr, nullptr, nullptr, &err_num);
    if (err_num != CL_SUCCESS) {
      std::cout << "failed to map image, err num: " << err_num << std::endl;
      return -1;
    }

    const std::string result_file = "out.jpg";
    auto ret = SaveImage(result_file, out_buffer, width, height);
    if (!ret) {
      std::cout << "failed to save image" << std::endl;
      return -1;
    }
    auto t_12 = Utils::GetNowUs();
    std::cout << "time(SaveImage): " << (t_12 - t_11) << " us" << std::endl;

    cout << "executing kernel successfully." << endl;
    queue.enqueueUnmapMemObject(out_image, out_buffer);

  } catch (cl::Error &e) {
    cerr << "catch cl error: " << e.what() << "(" << e.err() << ")" << endl;
  }
}







