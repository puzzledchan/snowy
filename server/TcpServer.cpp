#include "TcpServer.hpp"

TcpServer::TcpServer(EventLoop *loop, const std::string &listenAddd,
                     const std::string &nameArgs, Option option)
    : pool_(4) {
  currentLoop_ = 0;
  numLoop_ = 4;
}

TcpServer::~TcpServer() {}

EventLoop *TcpServer::BaseLoop() { return &loop_; }

EventLoop *TcpServer::GetNextLoop() {
  auto &loop = loops_[currentLoop_++ % loops_.size()];
  return loop.get();
}

void TcpServer::Listen(const SocketAddr &listenAddr, NewConnectionCallback cb,
                       BindCallback bfcb) {
  auto loop = BaseLoop();

  auto NewAcceptionFunc = [this, newCb = cb](int connfd, SocketAddr peer) {
    EventLoop *ioLoop = this->GetNextLoop();
    auto conn(std::make_shared<Connection>(ioLoop));
    // conn->Init(connfd, peer);
    if (ioLoop->Register(Poller::eET_Read, conn)) {
      newCb(conn.get());
      // conn->_OnConnect();
    } else {
      std::cout << "Register failed" << std::endl;
    }
  };
  loop->RunInThisLoop([loop, listenAddr, NewAcceptionFunc, bfcb]() {
    if (!loop->Listen(listenAddr, std::move(NewAcceptionFunc)))
      bfcb(false, listenAddr);
    else
      bfcb(true, listenAddr);
  });
}

void TcpServer::Listen(const char *ip, uint16_t hostPort,
                       NewConnectionCallback cb, BindCallback bfcb) {
  SocketAddr addr(ip, hostPort);
  Listen(addr, std::move(cb), std::move(bfcb));
}

void TcpServer::Run() {
  // start loops in thread pool
  _StartWorkers();
  BaseLoop()->Run();

  printf("Stopped BaseEventLoop...\n");

  loops_.clear();
  numLoop_ = 0;
  printf("Stopped WorkerEventLoops...\n");
}

void TcpServer::_StartWorkers() {
  // only called by main thread

  std::mutex mutex;
  std::condition_variable cond;

  for (size_t i = 0; i < numLoop_; ++i) {
    pool_.Submit([this, &mutex, &cond]() {
      EventLoop *loop(new EventLoop);

      {
        std::unique_lock<std::mutex> guard(mutex);
        loops_.push_back(std::unique_ptr<EventLoop>(loop));
        if (loops_.size() == numLoop_)
          cond.notify_one();
      }

      loop->Run();
    });
  }

  std::unique_lock<std::mutex> guard(mutex);
  cond.wait(guard, [this]() { return loops_.size() == numLoop_; });
}
