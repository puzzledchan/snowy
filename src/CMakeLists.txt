message(STATUS "Generating CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR} ")
set(LIB_SRC
    log/Logger.cpp
    utils/ByteArray.cpp
    utils/Buffer.cpp

    net/Acceptor.cpp
    net/Connection.cpp
    net/Epoller.cpp
    net/EventLoop.cpp
    net/TcpServer.cpp
    net/Socket.cpp

    rpc/RpcClient.cpp
    rpc/RpcServer.cpp
    rpc/RpcSession.cpp
)

set(Header
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/log
    ${CMAKE_CURRENT_SOURCE_DIR}/rpc
    ${CMAKE_CURRENT_SOURCE_DIR}/net
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
set(LIB_LIB Threads::Threads)

add_library(snowy SHARED ${LIB_SRC})
target_link_libraries(snowy PUBLIC ${LIB_LIB})
target_include_directories(snowy PUBLIC ${Header})