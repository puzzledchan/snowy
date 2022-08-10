#include <condition_variable>

#include "TcpServer.hpp"

TcpServer::TcpServer() {
  loop_.reset(new EventLoop(1));
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
    printf("loops_.size() = %d\n", loops_.size());
    auto acc = std::make_shared<Acceptor>(loop_, loops_);
    acc->BindAndListen();
    printf("pointer:%p=====start==============\n", acc.get());
    printf("pointer:%p=====start==============\n", acc->Identifier());
    loop_->Register(EPOLL_ET_Read, acc);
    printf("Register Listening...\n");
  };

  loop_->RunInThisLoop(func);
}

void TcpServer::Run() {
  //_StartWorkers();
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
  int numLoop = 8;
  for (size_t i = 0; i < numLoop; ++i) {
    auto func = [this, &pool_mutex, &cond, numLoop]() {
      auto loop = std::make_shared<EventLoop>(2);
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