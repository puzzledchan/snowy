/**
 * @file Poller.hpp
 * @author JDongChen
 * @brief IO多路复用
 * @version 0.1
 * @date 2022-08-06
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SNOWY_POLLER_H
#define SNOWY_POLLER_H

#include <sys/epoll.h>
#include <unistd.h>

#include <iostream>
#include <vector>

struct FiredEvent {
  int events;
  void *userdata; // use for channe pointer

  FiredEvent() : events(0), userdata(nullptr) {}
};

enum PollEvent {
  EPOLL_ET_None = 0,
  EPOLL_ET_Read = 0x1 << 0,
  EPOLL_ET_Write = 0x1 << 1,
  EPOLL_ET_ERROR = 0x1 << 2,
};

class Poller {
public:
  Poller() : multiplexer_(-1) {}

  virtual ~Poller() {}

  virtual bool Register(int fd, int events, void *userPtr) = 0;
  virtual bool Modify(int fd, int events, void *userPtr) = 0;
  virtual bool Unregister(int fd, int events) = 0;

  virtual int Poll(std::size_t maxEv, int timeoutMs) = 0;
  const std::vector<FiredEvent> &GetFiredEvents() const { return firedEvents_; }

public:
  int multiplexer_;
  std::vector<FiredEvent> firedEvents_;
};

class Epoller : public Poller {
public:
  Epoller();
  ~Epoller();

  Epoller(const Epoller &) = delete;
  void operator=(const Epoller &) = delete;

public:
  bool Register(int epfd, int events, void *userPtr) override;
  bool Modify(int epfd, int events, void *userPtr) override;
  bool Unregister(int epfd, int events) override;
  int Poll(std::size_t maxEvent, int timeoutMs) override;

private:
  std::vector<epoll_event> events_;
};

#endif