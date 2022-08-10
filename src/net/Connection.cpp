#include "Connection.hpp"
#include "EventLoop.hpp"
#include "socket.hpp"
#include <iostream>

Connection::Connection(std::shared_ptr<EventLoop> loop)
    : loop_(loop), local_sock_(kInvalid_) {}

bool Connection::Init(int sock, const sockaddr_in &peer) {
  if (sock == kInvalid_)
    return false;

  local_sock_ = sock;
  peer_ = peer;
  SetNonBlock(local_sock_);
  return true;
}

int Connection::Identifier() const { return local_sock_; }
bool Connection::HandleReadEvent() {
  std::cout << "HandleReadEvent" << std::endl;
  return true;
}

bool Connection::HandleWriteEvent() {
  std::cout << "HandleWriteEvent" << std::endl;
  return true;
}

void Connection::HandleErrorEvent() {
  std::cout << "HandleErrorEvent" << std::endl;
}