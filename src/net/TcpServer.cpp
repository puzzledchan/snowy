#include <arpa/inet.h>
#include <condition_variable>

#include "TcpServer.hpp"

TcpServer::TcpServer() {
  loop_.reset(new EventLoop());
  thread_pool_.clear();
}
TcpServer::~TcpServer() {
  loop_.reset();
  for (auto &thread : thread_pool_) {
    if (thread.joinable()) {
      thread.join();
    }
  }
}

void TcpServer::Listen() {
  auto func = [this]() {
    auto acc = std::make_shared<Acceptor>(loop_);
    auto newConnFunc = std::bind(&TcpServer::makeNewConnection, this,
                                 std::placeholders::_1, std::placeholders::_2);
    acc->setMakeNewConnection(newConnFunc);
    acc->BindAndListen();
    loop_->Register(EPOLL_ET_Read, acc);
  };

  loop_->RunInThisLoop(func);
}

void TcpServer::Start() {
  _StartWorkers();
  printf("start workers...\n");
  Listen();
  loop_->Run();
  printf("Stopped BaseEventLoop...\n");

  for (auto &thread : thread_pool_) {
    thread.join();
  }
  loops_.clear();
  printf("Stopped WorkerEventLoops...\n");
}

void TcpServer::_StartWorkers() {
  std::mutex pool_mutex;
  std::condition_variable cond;
  std::size_t numLoop = 8;
  for (size_t i = 0; i < numLoop; ++i) {
    auto func = [this, &pool_mutex, &cond, numLoop]() {
      auto loop = std::make_shared<EventLoop>();
      {
        std::unique_lock<std::mutex> guard(pool_mutex);
        loops_.push_back(loop);
        if (loops_.size() == numLoop)
          cond.notify_one();
      }
      loop->Run();
    };
    thread_pool_.emplace_back(std::thread(func));
  }

  std::unique_lock<std::mutex> guard(pool_mutex);
  cond.wait(guard, [this, numLoop]() { return loops_.size() == numLoop; });
}

std::shared_ptr<EventLoop> TcpServer::_getNextLoop() {
  printf("next loop_index: %ld\n", loops_.size());
  return loops_[next_loop_ind_++ % loops_.size()];
  ;
}

void TcpServer::makeNewConnection(int connfd, const sockaddr_in &peer) {
  // auto server = this;
  // auto loop = server->_getNextLoop();
  auto loop = _getNextLoop();
  auto func = [loop, connfd, peer]() {
    auto conn(std::make_shared<Connection>(loop));
    conn->Init(connfd, peer);
    loop->Register(EPOLL_ET_Read, conn);
  };
  loop->RunInThisLoop(func);
}