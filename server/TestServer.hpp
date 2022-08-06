#include "EventLoop.hpp"

class TestServer {
private:
  EventLoop loop_; // acceptor Loop

public:
  TestServer();
  ~TestServer();
  void Run() { loop_.Run(); }
};