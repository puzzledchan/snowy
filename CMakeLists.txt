cmake_minimum_required(VERSION 3.16)

# Set the project name
project(snowy)

set(CMAKE_CXX_COMPILER "/usr/bin/g++-11")
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic  -std=c++20  -Wall -Werror -Wno-deprecated -Wno-unused-function ")

# set the executable and library path
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 引入utils，重定义宏无警告
include(cmake/utils.cmake)
add_definitions(-Wno-builtin-macro-redefined)

# Add sub directories
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(examples)
