#include "Acceptor.hpp"
#include "EventLoop.hpp"

class TcpServer {
private:
  const std::string ipPort_;
  const std::string name_;
  std::atomic<size_t> next_loop_ind_{0};

public:
  std::vector<std::shared_ptr<EventLoop>> loops_;
  std::vector<std::thread> thread_pool_;
  std::shared_ptr<EventLoop> loop_;

public:
  TcpServer();
  ~TcpServer();
  void Start();
  void Listen();

  virtual void makeNewConnection(int connfd, const sockaddr_in &peer);

private:
  void _StartWorkers();

protected:
  std::shared_ptr<EventLoop> _getNextLoop();

  // void _Listen();
};