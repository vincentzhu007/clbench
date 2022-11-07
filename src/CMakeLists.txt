
find_package(OpenCL REQUIRED)

add_executable(hello_cl)
target_sources(hello_cl PRIVATE hello_cl.cpp)
target_link_libraries(hello_cl PRIVATE OpenCL::OpenCL)

set_target_properties(hello_cl PROPERTIES C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF)
target_compile_definitions(hello_cl PRIVATE CL_TARGET_OPENCL_VERSION=100)


add_executable(vector_add)
target_sources(vector_add PRIVATE vector_add_c_api.cpp)
target_link_libraries(vector_add PRIVATE OpenCL::OpenCL)

set_target_properties(vector_add PROPERTIES C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF)
target_compile_definitions(vector_add PRIVATE CL_TARGET_OPENCL_VERSION=100)

add_executable(vector_add_cpp)
target_sources(vector_add_cpp PRIVATE vector_add_cxx_api.cpp)
target_link_libraries(vector_add_cpp PRIVATE OpenCL::OpenCL)

set_target_properties(vector_add_cpp PROPERTIES C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF)
target_compile_definitions(vector_add_cpp PRIVATE CL_TARGET_OPENCL_VERSION=100)
target_compile_definitions(vector_add_cpp PRIVATE CL_HPP_MINIMUM_OPENCL_VERSION=100)
target_compile_definitions(vector_add_cpp PRIVATE CL_HPP_TARGET_OPENCL_VERSION=100)
target_compile_definitions(vector_add_cpp PRIVATE CL_HPP_ENABLE_EXCEPTIONS)
target_compile_definitions(vector_add_cpp PRIVATE CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY)


add_executable(time_vector_add_cpp)
target_sources(time_vector_add_cpp PRIVATE time_vector_add_cxx_api.cpp utils.cpp)
target_link_libraries(time_vector_add_cpp PRIVATE OpenCL::OpenCL)

set_target_properties(time_vector_add_cpp PROPERTIES C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF)
target_compile_definitions(time_vector_add_cpp PRIVATE CL_TARGET_OPENCL_VERSION=100)
target_compile_definitions(time_vector_add_cpp PRIVATE CL_HPP_MINIMUM_OPENCL_VERSION=100)
target_compile_definitions(time_vector_add_cpp PRIVATE CL_HPP_TARGET_OPENCL_VERSION=100)
target_compile_definitions(time_vector_add_cpp PRIVATE CL_HPP_ENABLE_EXCEPTIONS)
target_compile_definitions(time_vector_add_cpp PRIVATE CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY)