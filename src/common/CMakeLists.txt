add_library(cl_runtime)
target_sources(cl_runtime PRIVATE cl_runtime.cpp)
target_include_directories(cl_runtime INTERFACE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(cl_runtime PUBLIC OpenCL::OpenCL)
