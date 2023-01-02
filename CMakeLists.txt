cmake_minimum_required(VERSION 3.12)

project(CLBench LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#include(cmake/freeimage.cmake)

include(CTest)
enable_testing()

add_subdirectory(third_party)
add_subdirectory(src)
add_subdirectory(test)
