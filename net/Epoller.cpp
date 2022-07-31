/**
 * @file Epoller.cpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-07-31
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "Epoller.hpp"
#include <cassert>
#include <errno.h>
#include <unistd.h>

Epoller::Epoller() { multiplexer_ = ::epoll_create(512); }

Epoller::~Epoller() {
  if (multiplexer_ >= 0) {
    ::close(multiplexer_);
  }
}

int Epoller::Poll(size_t maxEvent, int timeoutMs) {
  if (maxEvent == 0)
    return 0;
  while (events_.size() < maxEvent) {
    events_.resize(2 * events_.size() + 1);
  }
  int nFired = ::epoll_wait(multiplexer_, &events_[0], maxEvent, timeoutMs);
  if (nFired == -1 && errno != EINTR && errno != EWOULDBLOCK)
    return -1;

  auto &events = firedEvents_;
  for (int i = 0; i < nFired; ++i) {
    FiredEvent &fired = events[i];
    fired.events = 0;
    fired.userdata = events_[i].data.ptr;

    if (events_[i].events & EPOLLIN)
      fired.events |= eET_Read;

    if (events_[i].events & EPOLLOUT)
      fired.events |= eET_Write;

    if (events_[i].events & (EPOLLERR | EPOLLHUP))
      fired.events |= eET_Error;
  }

  return nFired;
}

bool Epoller::Register(int fd, int events, void *userPtr) {
  if (fd < 0)
    return false;
  epoll_event ev;
  ev.data.ptr = userPtr;
  ev.events = 0;
  if (events & eET_Read)
    ev.events |= EPOLLIN;
  if (events & eET_Write)
    ev.events |= EPOLLOUT;
  return 0 == epoll_ctl(multiplexer_, EPOLL_CTL_ADD, fd, &ev);
}

bool Epoller::Unregister(int fd, int events) {
  if (fd < 0)
    return false;
  epoll_event dummy;
  return 0 == epoll_ctl(multiplexer_, EPOLL_CTL_DEL, fd, &dummy);
}

bool Epoller::Modify(int fd, int events, void *userPtr) {
  if (fd < 0)
    return false;
  epoll_event ev;
  ev.data.ptr = userPtr;
  ev.events = 0;
  if (events & eET_Read)
    ev.events |= EPOLLIN;
  if (events & eET_Write)
    ev.events |= EPOLLOUT;

  return 0 == epoll_ctl(multiplexer_, EPOLL_CTL_MOD, fd, &ev);
}
