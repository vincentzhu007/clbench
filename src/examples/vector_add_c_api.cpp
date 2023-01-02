//
// Created by zgd on 10/31/22.
//

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#if __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

using namespace std;

constexpr int VECTOR_SIZE = 10;

cl_context CreateContext() {
  cl_int errCode;
  cl_uint numPlatforms;
  cl_platform_id firstPlatformId;
  cl_context context = nullptr;

  errCode = clGetPlatformIDs(1, &firstPlatformId, &numPlatforms);
  if (errCode != CL_SUCCESS) {
    cerr << "Get platform id failed." << endl;
    return nullptr;
  }

  cl_context_properties context_properties[] = {
    CL_CONTEXT_PLATFORM,
    (cl_context_properties)firstPlatformId,
    0
  };
  context = clCreateContextFromType(context_properties, CL_DEVICE_TYPE_GPU, nullptr, nullptr, &errCode);
  if (errCode != CL_SUCCESS) {
    cerr << "Could not create context of GPU, try CPU..." << endl;
    context = clCreateContextFromType(context_properties, CL_DEVICE_TYPE_CPU, nullptr, nullptr, &errCode);
    if (errCode != CL_SUCCESS) {
      cerr << "Create context failed." << endl;
      return nullptr;
    }
  }
  return context;
}

cl_command_queue CreateCommandQueue(cl_context context, cl_device_id *device) {
  cl_int errCode;
  cl_device_id *devices;
  cl_command_queue command_queue = nullptr;
  size_t deviceBufferSize = 1;

  errCode = clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, nullptr, &deviceBufferSize);
  if (errCode != CL_SUCCESS) {
    cerr << "Get device size from context failed." << endl;
    return nullptr;
  }
  if (deviceBufferSize <= 0) {
    cerr << "No device available." << endl;
    return nullptr;
  }

  devices = new cl_device_id[deviceBufferSize / sizeof(cl_device_id)];
  errCode = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceBufferSize, devices, nullptr);
  if (errCode != CL_SUCCESS) {
    cerr << "Get devices from context failed." << endl;
    return nullptr;
  }

  command_queue = clCreateCommandQueue(context, devices[0], 0, nullptr);
  if (command_queue == nullptr) {
    cerr << "Create command queue of device 0 failed." << endl;
    return nullptr;
  }
  *device = devices[0];
  delete[] devices;
  return command_queue;
}

cl_program CreateProgram(cl_context context, cl_device_id device, const char *filename) {
  cl_int errCode = CL_SUCCESS;
  cl_program program = nullptr;

  ifstream kernelStream(filename, ios::in);
  if (!kernelStream.is_open()) {
    cerr << "Failed to open kernel file: " << filename << endl;
    return nullptr;
  }
  ostringstream oss;
  oss << kernelStream.rdbuf();

  string srcStrBuf = oss.str();
  const char *srcStr = srcStrBuf.c_str();
  program = clCreateProgramWithSource(context, 1, (const char **)&srcStr, nullptr, &errCode);
  if (program == nullptr) {
    cerr << "Failed to create program with source: " << filename << endl;
    return nullptr;
  }

  errCode = clBuildProgram(program, 0, nullptr, nullptr, nullptr, nullptr);
  if (errCode != CL_SUCCESS) {
    cerr << "Error in kernel: " << errCode << endl;

    cl_build_status build_status;
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_STATUS, sizeof(build_status), &build_status, NULL);
    cerr << "build status: " << build_status << endl;

    char buildLog[16384];
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buildLog), buildLog, NULL);

    cerr << buildLog << endl;
    clReleaseProgram(program);
    return nullptr;
  }
  return program;
}

bool CreateMemoryObjects(cl_context context, cl_mem memObjects[3], float *a, float *b) {
  memObjects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                 sizeof(float) * VECTOR_SIZE, a,nullptr);
  memObjects[1] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                 sizeof(float) * VECTOR_SIZE, b,nullptr);
  memObjects[2] = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                 sizeof(float) * VECTOR_SIZE, nullptr,nullptr);
  if ((memObjects[0] == nullptr) || (memObjects[1] == nullptr) || (memObjects[2] == nullptr)) {
    cout << "Error creating mem objects" << endl;
    return false;
  }
  return true;
}

void Cleanup(cl_context context, cl_command_queue command_queue, cl_program program, cl_kernel kernel,
             cl_mem memObjects[3]) {
  clReleaseContext(context);
  clReleaseCommandQueue(command_queue);
  clReleaseProgram(program);
  clReleaseKernel(kernel);
  clReleaseMemObject(memObjects[0]);
  clReleaseMemObject(memObjects[1]);
  clReleaseMemObject(memObjects[2]);
}


int main() {
  cl_context context = nullptr;
  cl_device_id device = 0;
  cl_command_queue command_queue = nullptr;
  cl_program program = nullptr;
  cl_kernel kernel = nullptr;
  cl_mem memObjects[3] = {nullptr, nullptr, nullptr};

  context = CreateContext();
  if (context == nullptr) {
    cerr << "Create context failed" << endl;
    Cleanup(context, command_queue, program, kernel, memObjects);
    return EXIT_FAILURE;
  }

  command_queue = CreateCommandQueue(context, &device);
  if (command_queue == nullptr) {
    cerr << "Create command queue failed" << endl;
    Cleanup(context, command_queue, program, kernel, memObjects);
    return EXIT_FAILURE;
  }

  program = CreateProgram(context, device, "../../src/vector_add.cl");
  if (program == nullptr) {
    cerr << "Create program failed" << endl;
    Cleanup(context, command_queue, program, kernel, memObjects);
    return EXIT_FAILURE;
  }

  cl_int errKernel = CL_SUCCESS;
  kernel = clCreateKernel(program, "vector_add", &errKernel);
  if (kernel == nullptr) {
    cerr << "Create kernel failed, error: " << errKernel << endl;
    Cleanup(context, command_queue, program, kernel, memObjects);
    return EXIT_FAILURE;
  }

  // Create memory object
  float result[VECTOR_SIZE];
  float a[VECTOR_SIZE];
  float b[VECTOR_SIZE];
  for (int i = 0; i < VECTOR_SIZE; i++) {
    a[i] = i;
    b[i] = VECTOR_SIZE - i;
  }

  if (!CreateMemoryObjects(context, memObjects, a, b)) {
    Cleanup(context, command_queue, program, kernel, memObjects);
    return EXIT_FAILURE;
  }
  cl_int errCode = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memObjects[0]);
  errCode |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[1]);
  errCode |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &memObjects[2]);
  if (errCode != CL_SUCCESS) {
    cerr << "Set kernel args failed" << endl;
    Cleanup(context, command_queue, program, kernel, memObjects);
    return EXIT_FAILURE;
  }

  size_t globalWorkSize[1] = {VECTOR_SIZE};
  size_t localWorkSize[1] = {1};
  errCode = clEnqueueNDRangeKernel(command_queue, kernel, 1, nullptr, globalWorkSize, localWorkSize, 0, nullptr, nullptr);
  if (errCode != CL_SUCCESS) {
    cerr << "Enqueue kernel failed" << endl;
    Cleanup(context, command_queue, program, kernel, memObjects);
    return EXIT_FAILURE;
  }

  errCode = clEnqueueReadBuffer(command_queue, memObjects[2], CL_TRUE, 0, VECTOR_SIZE * sizeof(float), result, 0, nullptr, nullptr);
  if (errCode != CL_SUCCESS) {
    cerr << "Enqueue read buffer failed" << endl;
    Cleanup(context, command_queue, program, kernel, memObjects);
    return EXIT_FAILURE;
  }

  for (int i = 0; i < VECTOR_SIZE; i++) {
    cout << result[i] << " ";
  }
  cout << endl;
  cout << "Execute kernel successfully." << endl;
  Cleanup(context, command_queue, program, kernel, memObjects);
}