cmake_minimum_required(VERSION 3.16)

# Set the project name
project(snowy)

# set the executable and library path
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# Add sub directories
add_subdirectory(tests)
