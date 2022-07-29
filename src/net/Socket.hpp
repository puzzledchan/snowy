/**
 * @file socket.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-08-20
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SNOWY_SOCKET_H
#define SNOWY_SOCKET_H

#include "string.h"
#include <arpa/inet.h>
#include <cassert>
#include <fcntl.h>

///@brief Create tcp socket
int CreateTCPSocket();

void SetNonBlock(int sock, bool nonblock = true);

#endif