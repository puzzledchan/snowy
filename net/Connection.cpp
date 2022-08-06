#include "Connection.hpp"
#include <Logger.hpp>

#include <string>

Connection::Connection(EventLoop *loop) : loop_(loop), connfd_(-1) {}

bool Connection::HandleReadEvent() {
  std::string buf_;
  buf_.resize(1024);
  const ssize_t len = ::read(connfd_, &buf_, 1024);
  if (len > 0) {
    SNOWY_LOG_DEBUG(SNOWY_LOG_ROOT) << "READ SUCCESSFUL";
  }
}