/**
 * @file TcpServer.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-07-31
 *
 * @copyright Copyright (c) 2022
 *
 */

// !TODO Must be Acceptor
#include "Connection.hpp"
#include "Typedef.hpp"
#include "concurrency/ThreadPool.hpp"

#include <Channel.hpp>
#include <EventLoop.hpp>

#include <atomic>
#include <functional>
#include <memory>
#include <string>
class TcpServer {
private:
  EventLoop loop_; // acceptor Loop
  const std::string ipPort_;
  const std::string name_;
  size_t numLoop_{0}; // loop number
  std::atomic<int> currentLoop_;
  ThreadPool pool_;

public:
  std::vector<std::unique_ptr<EventLoop>> loops_;

public:
  using ThreadInitCallback = std::function<void(EventLoop *)>;
  enum Option {
    NoReusePort,
    ReusePort,
  };
  TcpServer();
  // !TODO net address
  TcpServer(EventLoop *loop, const std::string &listenAddr,
            const std::string &nameArg, Option option = NoReusePort);
  ~TcpServer();
  const std::string &ipPort() const { return ipPort_; }
  const std::string &name() const { return name_; }
  EventLoop *BaseLoop();
  void Run();

public:
  using BindCallback = std::function<void(bool succ, const SocketAddr &)>;

public:
  EventLoop *GetNextLoop();
  void Listen(const SocketAddr &listenAddr, NewConnectionCallback cb,
              BindCallback bfcb = _DefaultBindCallback);
  void Listen(const char *ip, uint16_t hostPort, NewConnectionCallback cb,
              BindCallback bfcb = _DefaultBindCallback);

private:
  static void _DefaultBindCallback(bool succ, const SocketAddr &);
  void _StartWorkers();
};