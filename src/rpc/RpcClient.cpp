#include "RpcClient.hpp"

void RpcClient::handleMethodResponse(std::shared_ptr<Protocol> response) {
  // 获取该调用结果的序列号
  uint32_t id = response->getSequenceId();
  std::lock_guard guard(cli_mutex_);
  // 查找该序列号的 Channel 是否还存在，如果不存在直接返回
  auto it = sessionHandle_.find(id);
  if (it == sessionHandle_.end()) {
    return;
  }
  // 通过序列号获取等待该结果的 Channel
  auto pm = it->second;
  // 对该 Channel 发送调用结果唤醒调用者
  pm->set_value(response);
}

void RpcClient::start() {
  _startWorkers();
  printf("start workers...\n");
  // loop_->Run();
  connect();
  printf("Stopped BaseEventLoop...\n");

  for (auto &thread : thread_pool_) {
    thread.detach();
  }
  // loops_.clear();
  printf("Stopped WorkerEventLoops...\n");
}

bool RpcClient::connect() {
  auto local_sock = CreateTCPSocket();
  struct sockaddr_in addr;
  char SERVER_IP[] = "127.0.0.1";
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(2468);
  addr.sin_addr.s_addr = inet_addr(SERVER_IP);
  ::connect(local_sock, (struct sockaddr *)&addr, sizeof(addr));
  auto loop = _getNextLoop();
  rpc_session_ = std::make_shared<RpcSession>(loop);
  rpc_session_->Init(local_sock, addr);
  auto func =
      std::bind(&RpcClient::handleMethodResponse, this, std::placeholders::_1);
  rpc_session_->sethandleMethodResponse(func);

  loop->Register(EPOLL_ET_Read | EPOLL_ET_Write, rpc_session_);
  return true;
}
void RpcClient::_startWorkers() {
  std::mutex pool_mutex;
  std::condition_variable cond;
  std::size_t numLoop = 1;
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

std::shared_ptr<EventLoop> RpcClient::_getNextLoop() {
  return loops_[0];
  ;
}