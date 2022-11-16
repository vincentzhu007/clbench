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
#include "utils.h"

using namespace std;

constexpr int VECTOR_SIZE = 10000;

int main()
{
  // 预备材料1：定义kernel源码
  std::string kernel_source = R"(
    __constant sampler_t smp_zero = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

    __kernel void crop(__read_only image2d_t src_data, __write_only image2d_t dst_data,
                       int4 in_shape, int4 out_shape, int4 offset) {
      int out_h = get_global_id(0);
      int out_w = get_global_id(1);


      int out_batch_idx = out_h / out_shape.y;
      int out_height_idx = out_h % out_shape.y;
      int in_batch_idx = out_batch_idx + offset.x;
      int in_height_idx = out_height_idx + offset.y;
      int in_h = in_batch_idx * in_shape.y + in_height_idx;

      int out_width_idx = (out_w * 4) / out_shape.w;
      int out_channel_idx = (out_w * 4) % out_shape.w;
      int in_width_idx = out_width_idx + offset.z;
      int in_channel_idx = out_channel_idx + offset.w;
      int in_w = in_width_idx * in_shape.w + in_channel_idx;

      printf("out_h:%d, out_w:%d -> out_b:%d,out_he:%d,in_b:%d,in_he:%d,in_h:%d -> out_w:%d,out_c:%d,in_w:%d,in_c:%d,in_w:%d\n",
        out_h, out_w, out_batch_idx, out_height_idx, in_batch_idx, in_height_idx, in_h,
        out_width_idx, out_channel_idx, in_width_idx, in_channel_idx, in_w);
      printf("in_shape.x = %d\n", in_shape.x);
      printf("in_shape.y = %d\n", in_shape.y);
      printf("in_shape.z = %d\n", in_shape.z);
      printf("in_shape.w = %d\n", in_shape.w);

      float4 res = read_imagef(src_data, smp_zero, (int2)(in_w / 4, in_h));
      write_imagef(dst_data, (int2)(out_w, out_h), res);
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
    float in_buffer[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    cl_int err_num = CL_SUCCESS;
    cl::ImageFormat image_format;
    image_format.image_channel_order = CL_RGBA;
    image_format.image_channel_data_type = CL_FLOAT;
    cl::Image2D in_image(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, // Create input image
                         image_format, 2, 2, 0, in_buffer, &err_num);
    if (err_num != CL_SUCCESS) {
      std::cerr << "failed to create input image, err_num: " << err_num << std::endl;
      return -1;
    }

    size_t out_height = 1;
    size_t out_width = 1;
    cl::Image2D out_image(context, CL_MEM_WRITE_ONLY, // Create output image
                          image_format, out_height, out_width, 0, nullptr, &err_num);
    if (err_num != CL_SUCCESS) {
      std::cerr << "failed to create output image, err_num: " << err_num << std::endl;
      return -1;
    }

    // 4、执行kernel
    cl::CommandQueue queue(context, devices[0], 0);
    cl::Kernel kernel(program, "crop");

    cl_int4 out_shape = {1, 1, 1, 4};
    cl_int4 in_shape = {1, 2, 2, 4};
//    cl_int4 offset = {0, 0, 0, 0}; // out:[0, 1, 2, 3]
//    cl_int4 offset = {0, 0, 1, 0}; // out:[4, 5, 6, 7]
//    cl_int4 offset = {0, 1, 0, 0}; // out:[8, 9, 10, 11]
    cl_int4 offset = {0, 1, 1, 0}; // out:[12, 13, 14, 15]

    kernel.setArg(0, in_image);
    kernel.setArg(1, out_image);
    kernel.setArg(2, in_shape);
    kernel.setArg(3, out_shape);
    kernel.setArg(4, offset);

    cl::NDRange globalWorkSize(out_height, out_width);
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalWorkSize, cl::NullRange);
    queue.finish();

    float *out_buffer = new float[out_width * out_height * 4];
    queue.enqueueReadImage(out_image, CL_TRUE, {0, 0, 0}, {out_height, out_width, 1}, 0, 0, out_buffer);

    for (int i = 0; i < out_width * out_height * 4; i++) {
      std::cout << "out[" << i << "] = " << out_buffer[i] << std::endl;
    }

    cout << "executing kernel successfully." << endl;

  } catch (cl::Error &e) {
    cerr << "catch cl error: " << e.what() << "(" << e.err() << ")" << endl;
  }
}