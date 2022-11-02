//
// Created by zgd on 10/31/22.
//
#define __CL_ENABLE_EXCEPTIONS

#include <iostream>
#include <vector>
#if __APPLE__
#define CL_HPP_TARGET_OPENCL_VERSION 100
#include <OpenCL/opencl.hpp>
#else
#include <CL/cl.h>
#endif

using namespace std;

constexpr int VECTOR_SIZE = 10;

int main()
{
  int a[VECTOR_SIZE];
  int b[VECTOR_SIZE];
  int add_result[VECTOR_SIZE];
  int sub_result[VECTOR_SIZE];
  for (int i = 0; i < VECTOR_SIZE; i++) {
    a[i] = i + 1;
    b[i] = i - 1;
  }

  static char srcKernel[] =
      "__kernel void vector_add_sub(__global const int *a, __global const int *b, __global int *add_result, __global int *sub_result) {\n"
      "    int id = get_global_id(0);\n"
      "    printf(\"a[%d]=%d,b[%d]=%d\\n\", id, a[id], id, b[id]);\n"
      "    add_result[id] = a[id] + b[id];\n"
      "    sub_result[id] = a[id] - b[id];\n"
      "}\n";

  try {
    std::vector<cl::Platform> platformList;
    cl::Platform::get(&platformList);

    cl_context_properties context_properties[] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)(platformList[0])(),
        0
    };

    cl::Context context(CL_DEVICE_TYPE_GPU, context_properties);

    std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

    cl::CommandQueue queue(context, devices[0], 0);

    cl::Program::Sources sources(1, std::make_pair(srcKernel, 0));

    cl::Program program(context, sources);

    program.build(devices);

    cl::Buffer aBuffer = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, VECTOR_SIZE * sizeof(int), a);
    cl::Buffer bBuffer = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, VECTOR_SIZE * sizeof(int), b);
    cl::Buffer addResultBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, VECTOR_SIZE * sizeof(int), nullptr);
    cl::Buffer subResultBuffer = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, VECTOR_SIZE * sizeof(int), sub_result);

    cl::Kernel kernel(program, "vector_add_sub");

    kernel.setArg(0, aBuffer);
    kernel.setArg(1, bBuffer);
    kernel.setArg(2, addResultBuffer);
    kernel.setArg(3, subResultBuffer);

    cl::NDRange globalWorkSize(VECTOR_SIZE);
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalWorkSize, cl::NullRange);

    queue.enqueueReadBuffer(addResultBuffer, CL_TRUE, 0, VECTOR_SIZE * sizeof(int), add_result);
    int *output = (int *)queue.enqueueMapBuffer(subResultBuffer, CL_TRUE, CL_MAP_READ,
                                                0, VECTOR_SIZE * sizeof(int));
    cout << "add result: ";
    for (int i = 0; i < VECTOR_SIZE; i++) {
      cout << add_result[i] << " ";
    }
    cout << endl;

    cout << "sub result: ";
    for (int i = 0; i < VECTOR_SIZE; i++) {
      cout << sub_result[i] << " ";
    }
    cout << endl;

    cout << "executing kernel successfully." << endl;
    queue.enqueueUnmapMemObject(subResultBuffer, output);
  } catch (cl::Error &e) {
    cerr << "catch cl error: " << e.what() << "(" << e.err() << ")" << endl;
  }
}