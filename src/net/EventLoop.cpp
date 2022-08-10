
#include <cassert>

#include "EventLoop.hpp"

static thread_local EventLoop *g_thisLoop = nullptr;

EventLoop::EventLoop(int id) : poller_(new Epoller) {
  assert(!g_thisLoop && "There must be only one EventLoop per thread");
  g_thisLoop = this;
  running_ = true;
  id_ = id;
  printf("EventLoop is %d", id);

  // todo notify_all
}
EventLoop::~EventLoop() { printf("EventLoop released %d", id_); }
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
    // _WakeUp();
  }
}

bool EventLoop::Register(int events, std::shared_ptr<Channel> src) {
  if (poller_->Register(src->Identifier(), events, src.get())) {
    return channelSet_.insert(src).second;
  }
  return false;
}
bool EventLoop::Modify(int events, std::shared_ptr<Channel> src) {
  assert(channelSet_.find(src) != channelSet_.end());
  return poller_->Modify(src->Identifier(), events, src.get());
}

void EventLoop::Unregister(int events, std::shared_ptr<Channel> src) {
  poller_->Unregister(src->Identifier(), events);
  channelSet_.erase(src);
}

void EventLoop::Run() {
  const std::chrono::milliseconds defaultPollTime(100);
  while (running_) {
    _Loop(defaultPollTime);
  }
  for (auto &kv : channelSet_) {
    poller_->Unregister(kv->Identifier(), EPOLL_ET_Read | EPOLL_ET_Write);
  }
  channelSet_.clear();
  poller_.reset();
}

bool EventLoop::_Loop(std::chrono::milliseconds timeout) {
  if (channelSet_.empty()) {
    std::this_thread::sleep_for(timeout);
    return false;
  }
  // TODO: process poller_
  const int ready = poller_->Poll(static_cast<int>(channelSet_.size()),
                                  static_cast<int>(timeout.count()));
  if (ready < 0) {
    return false;
  }
  const auto &fired = poller_->GetFiredEvents();
  for (int i = 0; i < ready; ++i) {
    printf("==========================\n");
    assert(fired[i].userdata != nullptr);
    auto src = (Channel *)(fired[i].userdata);
    printf("Identifier:%d======================\n", fired[i].events);
    if (fired[i].events & EPOLL_ET_Read) {
      printf("EPOLL_ET_Read\n");
      if (!src->HandleReadEvent()) {
        src->HandleErrorEvent();
      }
    }

    if (fired[i].events & EPOLL_ET_Write) {
      if (!src->HandleWriteEvent()) {
        src->HandleErrorEvent();
      }
    }

    if (fired[i].events & EPOLL_ET_ERROR) {
      std::cout << "EPOLL_ET_ERROR" << std::endl;
      src->HandleErrorEvent();
    }
  }

  // TODO: process function TRY try_lock
  if (pendingFunctors_.size() == 0)
    return true;
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
  return true;
}