/**
 * @file EventLoop.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-07-31
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SNOWY_EVENTLOOP_H
#define SNOWY_EVENTLOOP_H

#include "Channel.hpp"
#include "Epoller.hpp"
#include "SocketAddr.hpp"
#include "Typedef.hpp"
#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

class EventLoop {
private:
  bool running_;
  std::unique_ptr<Epoller> poller_;
  static thread_local unsigned int s_id;

public:
  EventLoop();
  ~EventLoop();
  EventLoop(const EventLoop &) = delete;
  void operator=(const EventLoop &) = delete;
  EventLoop(EventLoop &&) = delete;
  void operator=(EventLoop &&) = delete;

public:
  using Functor = std::function<void()>; //

  using ChannelList = std::vector<std::unique_ptr<Channel *>>;
  using ChannelSet = std::map<unsigned int, std::shared_ptr<Channel>>;

  void Run();
  bool EventLoop::Listen(const char *ip, uint16_t hostPort,
                         NewAcceptionCallback newConnCallback);
  bool EventLoop::Listen(const SocketAddr &listenAddr,
                         NewAcceptionCallback newConnCallback);
  bool IsRunningThisLoop() const;
  void RunInThisLoop(Functor cb);

public:
  bool Register(int events, std::shared_ptr<Channel> src);
  bool Modify(int events, std::shared_ptr<Channel> src);
  void Unregister(int events, std::shared_ptr<Channel> src);

private:
  bool _Loop(std::chrono::milliseconds timeout);
  void _QueueInThisLoop(Functor cb);
  void _WakeUp();
  std::vector<Functor> pendingFunctors_;
  std::atomic<bool> callingPendingFunctors_; /* atomic */
  std::mutex funcMutex_;
  int wakeupFd_; // just for help wakeup

  ChannelList activeChannels_; // activeChannels_ process
  ChannelSet channelSet_;
};

#endif /* SNOWY_EVENTLOOP_H */