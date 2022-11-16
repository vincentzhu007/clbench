//
// Created by zgd on 11/10/22.
//
#define __CL_ENABLE_EXCEPTIONS

#include <iostream>
#include <vector>
#if __APPLE__
#include <OpenCL/opencl.hpp>
#else
#include <CL/cl2.hpp>
#endif

using namespace std;

constexpr int VECTOR_SIZE = 10000;

int main()
{
  // 预备材料1：定义kernel源码
  std::string kernel_source = R"(
    __constant sampler_t smp_zero = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

    __kernel void print_image2d(__read_only image2d_t src_data, int4 in_shape) {
      int w = get_global_id(0);
      int h = get_global_id(1);
      float4 res = read_imagef(src_data, smp_zero, (int2)(w, h));
      printf("[w:%d,h:%d] = {%f, %f, %f, %f}\n", w, h, res.x, res.y, res.z, res.w);
    }
  )";

  try {
    // 1、查找平台和设备，创建上下文
    std::vector<cl::Platform> platformList;
    cl::Platform::get(&platformList);

    cl_context_properties context_properties[] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)(platformList[0])(),
        0
    };
    cl::Context context(CL_DEVICE_TYPE_GPU, context_properties);

    std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

    // 2、编译cl kernel程序
    cl::Program::Sources sources(1, std::make_pair(kernel_source.c_str(), 0));
    cl::Program program(context, sources);
    try {
      program.build(devices);
    } catch (cl::Error &e) {
      if(program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(devices[0]) == CL_BUILD_ERROR) {
        auto build_log = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]);
        std::cout << "build opencl error: " << build_log << endl;
      }
    }

    // 3、设置kernel输入输出参数
    size_t height = 2;
    size_t width = 3;
    cl_int4 in_shape = {1, 2, 3, 4};
    float in_buffer[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
                         13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};

    cl_int err_num = CL_SUCCESS;
    cl::ImageFormat image_format;
    image_format.image_channel_order = CL_RGBA;
    image_format.image_channel_data_type = CL_FLOAT;
    cl::Image2D in_image(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, // Create input image
                         image_format, width, height, 0, in_buffer, &err_num);
    if (err_num != CL_SUCCESS) {
      std::cerr << "failed to create input image, err_num: " << err_num << std::endl;
      return -1;
    }

    // 4、执行kernel
    cl::CommandQueue queue(context, devices[0], 0);
    cl::Kernel kernel(program, "print_image2d");

    kernel.setArg(0, in_image);
    kernel.setArg(1, in_shape);

    cl::NDRange globalWorkSize(width, height);
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalWorkSize, cl::NullRange);
    queue.finish();

    cout << "executing kernel successfully." << endl;

  } catch (cl::Error &e) {
    cerr << "catch cl error: " << e.what() << "(" << e.err() << ")" << endl;
  }
}