/**
 * @file Typedef.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-07-31
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SNOWY_TYPEDEF_H
#define SNOWY_TYPEDEF_H
#include "functional"

struct SocketAddr;
class Connection;
class DatagramSocket;
class EventLoop;

using NewConnectionCallback = std::function<void(Connection *)>;

using NewAcceptionCallback = std::function<void(int connfd, SocketAddr peer)>;
using TcpConnFailCallback =
    std::function<void(EventLoop *, const SocketAddr &peer)>;

#endif