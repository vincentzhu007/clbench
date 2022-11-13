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

//class ISimpleKernel {
// public:
//  void Run();
//
//};

int main()
{
  // 定义kernel源码
  std::string kernel_source = R"(
    #pragma OPENCL EXTENSION cl_khr_fp16 : enable

    __constant sampler_t smp_zero = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

    __kernel void crop(__read_only image2d_t src_data, __write_only image2d_t dst_data,
                       int4 in_shape, int4 out_shape, int4 offset) {
      int out_x = get_global_id(0);
      int out_y = get_global_id(1);

      int out_batch_idx = out_x / out_shape.y;
      int out_height_idx = out_x % out_shape.y;
      int in_batch_idx = out_batch_idx + offset.x;
      int in_height_idx = out_height_idx + offset.y;
      int in_x = in_batch_idx * in_shape.y + in_height_idx;

      int out_width_idx = out_y / out_shape.w;
      int out_channel_idx = out_y % out_shape.w;
      int in_width_idx = out_width_idx + offset.z;
      int in_channel_idx = out_channel_idx + offset.w;
      int in_y = in_width_idx * in_shape.w + in_channel_idx;

      float4 res = read_imagef(src_data, smp_zero, (int2)(in_x, in_y));
      write_imagef(dst_data, (int2)(out_x, out_y), res);
    }
  )";

  try {
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

    cl::CommandQueue queue(context, devices[0], 0);

    auto t_4 = Utils::GetNowUs();
    std::cout << "time(CommandQueue): " << (t_4 - t_3) << " us" << std::endl;

    cl::Program::Sources sources(1, std::make_pair(kernel_source, 0));

    auto t_5 = Utils::GetNowUs();
    std::cout << "time(Sources): " << (t_5 - t_4) << " us" << std::endl;

    cl::Program program(context, sources);

    auto t_6 = Utils::GetNowUs();
    std::cout << "time(Program): " << (t_6 - t_5) << " us" << std::endl;

    try {
      program.build(devices);
    } catch (cl::Error &e) {
      if(program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(devices[0]) == CL_BUILD_ERROR) {
        auto build_log = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]);
        std::cout << "build opencl error: " << build_log << endl;
      }
    }


    auto t_7 = Utils::GetNowUs();
    std::cout << "time(build): " << (t_7 - t_6) << " us" << std::endl;

    cl::Image2D src_image = cl::Image2D(context, )

    cl::Buffer aBuffer = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, VECTOR_SIZE * sizeof(int), a);
    cl::Buffer bBuffer = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, VECTOR_SIZE * sizeof(int), b);
    cl::Buffer addResultBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, VECTOR_SIZE * sizeof(int), nullptr);
    cl::Buffer subResultBuffer = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, VECTOR_SIZE * sizeof(int), sub_result);

    auto t_8 = Utils::GetNowUs();
    std::cout << "time(Buffer): " << (t_8 - t_7) << " us" << std::endl;

    cl::Kernel kernel(program, "crop");

//    kernel.setArg(0, aBuffer);
//    kernel.setArg(1, bBuffer);
//    kernel.setArg(2, addResultBuffer);
//    kernel.setArg(3, subResultBuffer);
//
//    auto t_9 = Utils::GetNowUs();
//    std::cout << "time(Kernel): " << (t_9 - t_8) << " us" << std::endl;
//
//    cl::NDRange globalWorkSize(VECTOR_SIZE);
//    queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalWorkSize, cl::NullRange);
//    queue.finish();
//
//    auto t_10 = Utils::GetNowUs();
//    std::cout << "time(enqueueNDRangeKernel): " << (t_10 - t_9) << " us" << std::endl;
//
//    queue.enqueueReadBuffer(addResultBuffer, CL_TRUE, 0, VECTOR_SIZE * sizeof(int), add_result);
//
//    auto t_11 = Utils::GetNowUs();
//    std::cout << "time(enqueueReadBuffer): " << (t_11 - t_10) << " us" << std::endl;
//
//    int *output = (int *)queue.enqueueMapBuffer(subResultBuffer, CL_TRUE, CL_MAP_READ,
//                                                0, VECTOR_SIZE * sizeof(int));
//
//    auto t_12 = Utils::GetNowUs();
//    std::cout << "time(enqueueMapBuffer): " << (t_12 - t_11) << " us" << std::endl;
//
//    cout << "executing kernel successfully." << endl;
//    queue.enqueueUnmapMemObject(subResultBuffer, output);

//  } catch (cl::Error &e) {
//    cerr << "catch cl error: " << e.what() << "(" << e.err() << ")" << endl;
//  }
}