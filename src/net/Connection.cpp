/**
 * @file Connection.cpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-08-20
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <cassert>
#include <iostream>

#include "Connection.hpp"

Connection::Connection(std::shared_ptr<EventLoop> loop)
    : loop_(loop), local_sock_(kInvalid_) {}

bool Connection::Init(int sock, const sockaddr_in &peer) {
  if (sock == kInvalid_)
    return false;

  local_sock_ = sock;
  peer_ = peer;
  SetNonBlock(local_sock_);
  assert(state_ == State::None);
  state_ = State::Connected;
  return true;
}

int Connection::Identifier() const { return local_sock_; }

bool Connection::HandleReadEvent() {
  if (state_ != State::Connected) {
    return false;
  }
  while (true) {
    recv_buf_.AssureSpace(1024);
    int bytes =
        ::recv(local_sock_, recv_buf_.writeAddr(), recv_buf_.writableSize(), 0);
    // nothing to read
    if (bytes == kInvalid_) {
      if (EAGAIN == errno || EWOULDBLOCK == errno)
        return true;
      if (EINTR == errno)
        continue; // restart ::recv
      _Shutdown(ShutdownMode::SM_BOTH);
      state_ = State::Error;
      return false;
    }
    /// peer disconnect read end of file
    if (bytes == 0) {
      if (send_buf_.empty()) {
        _Shutdown(ShutdownMode::SM_BOTH);
        state_ = State::PassiveClose;
      } else {
        state_ = State::CloseWaitWrite;
        _Shutdown(ShutdownMode::SM_READ);
        loop_->Modify(EPOLL_ET_Write, shared_from_this()); // disable read
      }
      return false;
    }
    if (bytes > 0) {
      // just echo
      recv_buf_.produce(bytes);
      processMessage();
    }
    if (send_buf_.readableSize() > 0) {
      loop_->Modify(EPOLL_ET_Write, shared_from_this());
    }
  }
  return true;
}

bool Connection::HandleWriteEvent() {
  while (send_buf_.readableSize() > 0) {
    int len =
        ::send(local_sock_, send_buf_.readAddr(), send_buf_.readableSize(), 0);
    send_buf_.consume(len);
  }
  loop_->Modify(EPOLL_ET_Read, shared_from_this());
  return true;
}

void Connection::HandleErrorEvent() {
  switch (state_) {
  case State::PassiveClose:
  case State::ActiveClose:
  case State::Error:
    break;
  case State::None:
  case State::CloseWaitWrite:
  case State::Closed:
  default:
    return;
  }
  state_ = State::Closed;
  loop_->Unregister(EPOLL_ET_Read | EPOLL_ET_Write, shared_from_this());
}

void Connection::_Shutdown(ShutdownMode mode) {
  switch (mode) {
  case ShutdownMode::SM_READ:
    ::shutdown(local_sock_, SHUT_RD);
    break;
  case ShutdownMode::SM_WRITE:
    ::shutdown(local_sock_, SHUT_RDWR);
    break;
  case ShutdownMode::SM_BOTH:
    ::shutdown(local_sock_, SHUT_RDWR);
    break;
  }
}

// just echo and send
void Connection::processMessage() {
  printf("Connection::processMessage()\n");
  std::string buf;
  buf.resize(recv_buf_.readableSize());
  recv_buf_.popData(&buf[0], buf.size());
  std::cout << buf << std::endl;
  send_buf_.pushData(&buf[0], buf.size());
}