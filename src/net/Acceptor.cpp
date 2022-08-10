#include "fcntl.h"

#include "Acceptor.hpp"
#include "Connection.hpp"
#include "EventLoop.hpp"
#include "Poller.hpp"

int Acceptor::Identifier() const { return local_sock_; }

void Acceptor::BindAndListen() {
  struct sockaddr_in addr;

  local_port_ = 1230;
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
  ret = ::listen(local_sock_, 1024);
  fcntl(local_sock_, F_SETFL, fcntl(local_sock_, F_GETFD, 0) | O_NONBLOCK);
}

bool Acceptor::HandleReadEvent() {
  while (true) {
    int connfd = _Accept();
    if (connfd != kInvaild_) {
      std::shared_ptr<EventLoop> loop;
      if (loop_pool_.size()) {
        loop = loop_pool_[++current_loop_ind_ % loop_pool_.size()];
      } else {
        loop = baseloop_;
      }
      auto func = [loop, connfd, peer = peer_]() {
        auto conn(std::make_shared<Connection>(loop));
        conn->Init(connfd, peer);
        if (loop->Register(EPOLL_ET_Read, conn))
          printf("success connfd\n");
      };
      loop->RunInThisLoop(func);
    }
  }
}

bool Acceptor::HandleWriteEvent() {
  assert(false);
  return false;
}

void Acceptor::HandleErrorEvent() {
  assert(false);
  return;
}