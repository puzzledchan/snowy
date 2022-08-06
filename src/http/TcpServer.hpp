#include "../net/EventLoop.hpp" 
#include "../net/Acceptor.hpp"


class TcpServer {
private:
  const std::string ipPort_;
  const std::string name_;

public:
  std::vector<std::shared_ptr<EventLoop>> loops_;
  std::vector<std::thread> thread_pool_;
  std::shared_ptr<EventLoop> loop_;
public:
    TcpServer();
    ~TcpServer();
    void Run();
    void Listen();

private:
    void _StartWorkers();
    //void _Listen();
};