include(GoogleTest)

add_executable(hello_gtest hello_gtest.cpp)
target_link_libraries(hello_gtest PRIVATE gtest gtest_main)
gtest_discover_tests(hello_gtest)