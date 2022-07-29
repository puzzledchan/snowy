#include "fcntl.h"

#include "Acceptor.hpp"
#include "EventLoop.hpp"
#include "Poller.hpp"

int Acceptor::Identifier() const { return local_sock_; }

void Acceptor::BindAndListen() {
  struct sockaddr_in addr;

  local_port_ = 2468;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(local_port_);
  // create TCP socket
  local_sock_ = socket(AF_INET, SOCK_STREAM, 0);
  if (local_sock_ < 0) {
    printf("failed create TCP listen\n");
  } else {
    printf("create TCP listen success\n");
  }
  int ret = ::bind(local_sock_, (struct sockaddr *)&addr, sizeof(addr));
  if (ret < 0)
    assert(false);
  ret = ::listen(local_sock_, 1024);
  fcntl(local_sock_, F_SETFL, fcntl(local_sock_, F_GETFD, 0) | O_NONBLOCK);
}

bool Acceptor::HandleReadEvent() {
  while (true) {
    int connfd = _Accept();
    if (connfd != kInvaild_) {
      // 多态创建新线程
      makeNewConnection(connfd, peer_);
    } else {
      bool goAhead = false;
      const int error = errno;
      switch (error) {
      case EAGAIN:
        return true; // it is fine for nonblock
      case EINTR:
      case ECONNABORTED:
      case EPROTO:
        goAhead = true; // should retry
        break;
      case EMFILE:
      case ENFILE:
        return true;
      case ENOBUFS:
      case ENOMEM:
        return true; // not enough memory
      case ENOTSOCK:
      case EOPNOTSUPP:
      case EINVAL:
      case EFAULT:
      case EBADF:
      default:
        assert(false);
        break;
      }
      if (!goAhead)
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
