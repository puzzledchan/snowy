/**
 * @file Acceptor.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-07-31
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SNOWY_ACCEPTOR_H
#define SNOWY_ACCEPTOR_H

#include "Channel.hpp"
#include "Connection.hpp"
#include "EventLoop.hpp"
#include "SocketAddr.hpp"
#include "netinet/in.h"

class EventLoop;
class Poller;
class Acceptor : public Channel {
private:
  using EventLoopPool = std::vector<std::shared_ptr<EventLoop>>;
  using EventLoopPoolPtr = std::shared_ptr<EventLoopPool>;
  using EventLoopPtr = std::shared_ptr<EventLoop>;
  EventLoopPtr const loop_;     // which loop belong to
  EventLoopPoolPtr const pool_; // the reactor pool
  int accfd_;
  sockaddr_in peer_;

public:
  explicit Acceptor(EventLoopPtr loop, EventLoopPoolPtr pool = nullptr);
  ~Acceptor();
  Acceptor(const Acceptor &) = delete;
  void operator=(const Acceptor &) = delete;

  bool HandleReadEvent() override;
  bool HandleWriteEvent() override;
  void HandleErrorEvent() override;

public:
  bool BindAddr(const SocketAddr &addr);

private:
  int _Accept();
};

#endif // Connected