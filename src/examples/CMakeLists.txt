
find_package(OpenCL REQUIRED)

add_compile_definitions(CL_TARGET_OPENCL_VERSION=210)
add_compile_definitions(CL_HPP_TARGET_OPENCL_VERSION=210)
add_compile_definitions(CL_HPP_ENABLE_EXCEPTIONS)
add_compile_definitions(CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY=1)

add_executable(hello_cl)
target_sources(hello_cl PRIVATE examples/hello_cl.cpp)
target_link_libraries(hello_cl PRIVATE OpenCL::OpenCL)

set_target_properties(hello_cl PROPERTIES C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF)
target_compile_definitions(hello_cl PRIVATE CL_TARGET_OPENCL_VERSION=100)


add_executable(vector_add)
target_sources(vector_add PRIVATE examples/vector_add_c_api.cpp)
target_link_libraries(vector_add PRIVATE OpenCL::OpenCL)

set_target_properties(vector_add PROPERTIES C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF)
target_compile_definitions(vector_add PRIVATE CL_TARGET_OPENCL_VERSION=100)

add_executable(vector_add_cpp)
target_sources(vector_add_cpp PRIVATE examples/vector_add_cxx_api.cpp)
target_link_libraries(vector_add_cpp PRIVATE OpenCL::OpenCL)

set_target_properties(vector_add_cpp PROPERTIES C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF)

add_executable(time_vector_add_cpp)
target_sources(time_vector_add_cpp PRIVATE examples/time_vector_add_cxx_api.cpp examples/utils.cpp)
target_link_libraries(time_vector_add_cpp PRIVATE OpenCL::OpenCL)

set_target_properties(time_vector_add_cpp PROPERTIES C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF)

add_executable(opencl_offical_demo)
target_sources(opencl_offical_demo PRIVATE examples/opencl_offical_demo.cpp)
target_link_libraries(opencl_offical_demo PRIVATE OpenCL::OpenCL)
set_target_properties(opencl_offical_demo PROPERTIES C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF)

add_executable(crop)
target_sources(crop PRIVATE examples/crop.cpp examples/utils.cpp)
target_link_libraries(crop PRIVATE OpenCL::OpenCL)
set_target_properties(crop PROPERTIES C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF)

add_executable(image2d_filter)
target_sources(image2d_filter PRIVATE examples/image2d_filter.cpp examples/utils.cpp)
target_link_libraries(image2d_filter PRIVATE OpenCL::OpenCL)
target_link_libraries(image2d_filter PRIVATE freeimage)
set_target_properties(image2d_filter PROPERTIES C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF)
configure_file(examples/test.jpg ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(examples/Lenna.jpg ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_executable(test_image2d_load_order)
target_sources(test_image2d_load_order PRIVATE examples/test_image2d_load_order.cpp)
target_link_libraries(test_image2d_load_order PRIVATE OpenCL::OpenCL)
set_target_properties(test_image2d_load_order PROPERTIES C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF)