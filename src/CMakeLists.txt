cmake_minimum_required(VERSION 3.10)

# Set the project name
project(snowy)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(server main.cpp)
aux_source_directory(http source)
aux_source_directory(net source)
target_sources(server PUBLIC ${source})

target_include_directories(server
    PUBLIC ${PROJECT_SOURCE_DIR}/utils/sysapi
)
target_link_directories(server
    PUBLIC ${PROJECT_SOURCE_DIR}/net
)
message(STATUS "PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")
message(STATUS "PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
message(STATUS "CMAKE_PROJECT_DIR: ${CMAKE_PROJECT_DIR}")

target_link_libraries(server PUBLIC Threads::Threads)