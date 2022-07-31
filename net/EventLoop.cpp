/**
 * @file EventLoop.cpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-07-31
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "EventLoop.hpp"
#include <Acceptor.hpp>
#include <cassert>
#include <sys/eventfd.h>
#include <unistd.h>

static thread_local EventLoop *g_thisLoop = nullptr;
thread_local unsigned int EventLoop::s_id = 0; // manager

int createEventfd() {
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    assert("wrong write count");
  }
  return evtfd;
}

EventLoop::EventLoop() {
  wakeupFd_ = createEventfd();
  // wakeupChannel_ = new Channel(this, wakeupFd_);
}

bool EventLoop::IsRunningThisLoop() const { return this == g_thisLoop; }

void EventLoop::RunInThisLoop(Functor cb) {
  if (IsRunningThisLoop()) {
    cb();
  } else {
    _QueueInThisLoop(std::move(cb));
  }
}

void EventLoop::_QueueInThisLoop(Functor cb) {
  {
    std::lock_guard<std::mutex> lock(funcMutex_);
    pendingFunctors_.emplace_back(std::move(cb));
  }
  if (!IsRunningThisLoop() || callingPendingFunctors_) {
    _WakeUp();
  }
}

void EventLoop::_WakeUp() {
  uint64_t one = 1;
  ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    assert("wrong write count");
  }
}

bool EventLoop::Listen(const char *ip, uint16_t hostPort,
                       NewAcceptionCallback newAccCallback) {
  SocketAddr addr;
  addr.Init(ip, hostPort);

  return Listen(addr, std::move(newAccCallback));
}
bool EventLoop::Listen(const SocketAddr &listenAddr,
                       NewAcceptionCallback newAccCallback) {
  auto s = std::make_shared<Acceptor>(this); // 进程绑定
  s->SetNewAcceptionCallback(std::move(newAccCallback));
  if (!s->BindAddr(listenAddr))
    return false;

  return true;
}

void EventLoop::Run() {
  const std::chrono::milliseconds defaultPollTime(10);

  while (running_) {
    _Loop(defaultPollTime);
  }
  for (auto &kv : channelSet_) {
    poller_->Unregister(Poller::eET_Read | Poller::eET_Write,
                        kv.second->Identifier());
  }

  channelSet_.clear();
  poller_.reset();
}

bool EventLoop::_Loop(const std::chrono::milliseconds timeout) {
  assert(!running_);
  if (channelSet_.empty()) {
    std::this_thread::sleep_for(timeout);
    return false;
  }

  // TODO: process poller_
  const int firedNum = poller_->Poll(static_cast<int>(channelSet_.size()),
                                     static_cast<int>(timeout.count()));
  if (firedNum < 0) {
    return false;
  }
  const auto &fired = poller_->GetFiredEvents();
  std::vector<std::shared_ptr<Channel>> sources(firedNum);
  for (int i = 0; i < firedNum; ++i) {
    auto src = (Channel *)fired[i].userdata;
    sources[i] = src->shared_from_this();
    if (fired[i].events & Poller::eET_Read) {
      if (!src->HandleReadEvent()) {
        src->HandleErrorEvent();
      }
    }

    if (fired[i].events & Poller::eET_Write) {
      if (!src->HandleWriteEvent()) {
        src->HandleErrorEvent();
      }
    }

    if (fired[i].events & Poller::eET_Error) {
      std::cout << "Error: " << std::endl;
      src->HandleErrorEvent();
    }
  }

  // TODO: process function
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;
  {
    std::lock_guard<std::mutex> guard(funcMutex_);
    functors.swap(pendingFunctors_);
  }
  for (const Functor &functor : functors) {
    functor();
  }
  callingPendingFunctors_ = false;
}

bool EventLoop::Register(int events, std::shared_ptr<Channel> src) {
  if (events == 0)
    return false;

  if (src->GetUniqueId() != 0)
    assert(false);

  /* man getrlimit:
   * RLIMIT_NOFILE
   * Specifies a value one greater than the maximum file descriptor number
   * that can be opened by this process.
   * Attempts (open(2), pipe(2), dup(2), etc.)
   * to exceed this limit yield the error EMFILE.
   */
  /*   if (src->Identifier() + 1 >= static_cast<int>(s_maxOpenFdPlus1)) {

      return false;
    }
   */
  ++s_id;
  if (s_id == 0) // wrap around
    s_id = 1;

  src->SetUniqueId(s_id);

  if (poller_->Register(src->Identifier(), events, src.get()))
    return channelSet_.insert({src->GetUniqueId(), src}).second;

  return false;
}

bool EventLoop::Modify(int events, std::shared_ptr<Channel> src) {
  assert(channelSet_.count(src->GetUniqueId()));
  return poller_->Modify(src->Identifier(), events, src.get());
}

void EventLoop::Unregister(int events, std::shared_ptr<Channel> src) {
  const int fd = src->Identifier();
  poller_->Unregister(fd, events);

  bool suc = channelSet_.erase(src->GetUniqueId());
  if (!suc) {
    assert(false);
  }
}
