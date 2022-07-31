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
  EventLoop *const loop_; // which loop belong to

  sockaddr_in peer_;
  int localSock_;
  uint16_t localPort_;

public:
  explicit Acceptor(EventLoop *loop);
  ~Acceptor();
  Acceptor(const Acceptor &) = delete;
  void operator=(const Acceptor &) = delete;

  bool HandleReadEvent() override;
  bool HandleWriteEvent() override;
  void HandleErrorEvent() override;

public:
  void SetNewAcceptionCallback(NewAcceptionCallback cb);
  bool BindAddr(const SocketAddr &addr);

private:
  int _Accept();
  NewAcceptionCallback newAccCallback_;
  static const int kListenQueue;
};

#endif // Connected