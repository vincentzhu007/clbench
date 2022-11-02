# clbench

OpenCL Benchmark工具集

## 如何构建？

### Ubuntu 20.04

*coming soon...*

### MAC OS

1、依赖包

MACOS的XCode开发工具自带OpenCL C库和头文件，但缺少C++头文件。

需拷贝 [opencl.hpp](https://github.com/KhronosGroup/OpenCL-CLHPP/blob/main/include/CL/opencl.hpp) 到系统的OpenCL库目录下。

```c++
sudo cp opencl.hpp /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk/System/Library/Frameworks/OpenCL.framework/Headers/
```
以上路径根据MACOS版本可能存在差异，具体查询OpenCL库路径的方法是：使用CLion IDE智能索引`vector_add_c_api.cpp`，点击`cl.h`头文件跳转，右键拷贝绝对路径即可。

2、编译运行

```shell
cd clbench
mkdir build & cd build
cmake ..
make

cd src
./hello_cl
./vector_add
./vector_add_cpp
```

如果用默认的AppleClang编译器可能会报错，建议使用`-DCMAKE_CXX_COMPILER`指定gnu编译器。

