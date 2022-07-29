set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(threadpool ${CMAKE_CURRENT_SOURCE_DIR}/threadpool/)
target_include_directories(threadpool
    PUBLIC ${PROJECT_SOURCE_DIR}/threadpool
)

target_link_libraries(threadpool PUBLIC Threads::Threads)
