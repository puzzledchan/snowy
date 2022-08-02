set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# set_target_properties(threadpool PROPERTIES LINKER_LANGUAGE CXX)
#[[
add_executable(threadpool ${CMAKE_CURRENT_SOURCE_DIR}/concurrency/test.cpp)
target_include_directories(threadpool
    PUBLIC ${PROJECT_SOURCE_DIR}/concurrency
)
target_link_libraries(threadpool PUBLIC Threads::Threads)

add_executable(singleton ${CMAKE_CURRENT_SOURCE_DIR}/utils/test_singleton.cpp)
target_include_directories(singleton
    PUBLIC ${PROJECT_SOURCE_DIR}/utils
)

add_executable(log ${CMAKE_CURRENT_SOURCE_DIR}/test_log/log_test.cpp ${PROJECT_SOURCE_DIR}/logger/log.cpp
    ${PROJECT_SOURCE_DIR}/utils/Timer.cpp ${PROJECT_SOURCE_DIR}/utils/Buffer.cpp)
target_include_directories(log
    PUBLIC ${PROJECT_SOURCE_DIR}/logger ${PROJECT_SOURCE_DIR}/utils
)
target_link_libraries(log PUBLIC Threads::Threads)
]]
add_executable(log ${CMAKE_CURRENT_SOURCE_DIR}/test_log/logger_test.cpp ${PROJECT_SOURCE_DIR}/logger/Logger.cpp
    ${PROJECT_SOURCE_DIR}/utils/SystemCall.cpp)
target_include_directories(log
    PUBLIC ${PROJECT_SOURCE_DIR}/logger ${PROJECT_SOURCE_DIR}/utils
)
target_link_libraries(log PUBLIC Threads::Threads)
