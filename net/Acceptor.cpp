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

const int Acceptor::kListenQueue = 1024;

Acceptor::Acceptor(EventLoop *loop)
    : localSock_(-1), localPort_(-1), loop_(loop) {}

Acceptor::~Acceptor() {
  if (localSock_ >= 0) {
    {
      ::close(localSock_);
      localSock_ = -1;
    }

    std::cout << "~Acceptor destruct" << std::endl;
  }
}

void Acceptor::SetNewAcceptionCallback(NewAcceptionCallback cb) {
  newAccCallback_ = std::move(cb);
}

bool Acceptor::BindAddr(const SocketAddr &addr) {
  if (!addr.IsValid())
    return false;

  if (localSock_ != kInvalid) {
    return false;
  }

  localSock_ = CreateTCPSocket();
  if (localSock_ == kInvalid)
    return false;

  localPort_ = addr.GetPort();

  SetNonBlock(localSock_);
  SetNodelay(localSock_);
  SetReuseAddr(localSock_);
  SetRcvBuf(localSock_);
  SetSndBuf(localSock_);

  auto serv = addr.GetAddr();

  int ret = ::bind(localSock_, (struct sockaddr *)&serv, sizeof serv);
  if (kError == ret) {
    return false;
  }

  ret = ::listen(localSock_, kListenQueue);
  if (kError == ret) {
    return false;
  }

  if (!loop_->Register(Poller::eET_Read, this->shared_from_this()))
    return false;

  return true;
}

bool Acceptor::HandleReadEvent() {
  while (true) {
    int connfd = _Accept();
    if (connfd >= 0) {
      // newConnCallback_();
      if (newAccCallback_) {
        newAccCallback_(connfd, peer_);
      }
    } else {
      // TODO: handle error
      return false;
    }
  }
  return true;
}
bool Acceptor::HandleWriteEvent() {
  assert(false);
  return false;
}

void Acceptor::HandleErrorEvent() {
  assert(false);
  return;
}

int Acceptor::_Accept() {
  socklen_t addrLength = sizeof(peer_);
  return ::accept(localSock_, (struct sockaddr *)&peer_, &addrLength);
}