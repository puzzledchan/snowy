/**
 * @file Acceptor.cpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-07-31
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "Acceptor.hpp"

#include <iostream>

#include <cassert>
#include <fcntl.h>  //   nonblocking
#include <unistd.h> // close socket

Acceptor::Acceptor(EventLoopPtr loop, EventLoopPoolPtr pool)
    : accfd_(-1), loop_(loop), pool_(pool) {}

bool Acceptor::HandleReadEvent() {
  while (true) {
    int connfd = _Accept();
    if (connfd != -1) {
      std::cout << "Accept Failed " << std::endl;
    }
    auto func = []() { std::cout << "Accept" << std::endl; };
    if (pool_ == nullptr)
      loop_->RunInThisLoop(func);
  }
}

bool Acceptor::HandleWriteEvent() {
  assert(false);
  return false;
}

void Acceptor::HandleErrorEvent() { assert(false); }

int Acceptor::_Accept() {
  socklen_t addrLength = sizeof(peer_);
  return ::accept(accfd_, (struct sockaddr *)&peer_, &addrLength);
}