
#include <cassert>

#include "EventLoop.hpp"

static thread_local EventLoop *g_thisLoop = nullptr;

EventLoop::EventLoop() {
    assert (!g_thisLoop && "There must be only one EventLoop per thread");
    g_thisLoop = this;
    running_ = true;
    poller_.reset(new Epoller);
    // todo notify_all
}
EventLoop::~EventLoop() {}
bool EventLoop::IsRunningThisLoop() const { return this == g_thisLoop; }
void EventLoop::RunInThisLoop(Functor cb) {
  if (IsRunningThisLoop()) {
    printf("start Loop in running_");
    cb();
  } else {
        printf("Queue functors in running_");
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
   printf("start registe\n");
    if(poller_->Register(src->Identifier(), events, src.get()))
        return channelSet_.insert(src).second;
    return false;
}
bool EventLoop::Modify(int events, std::shared_ptr<Channel> src) {
    assert (channelSet_.find(src)!= channelSet_.end());
    return poller_->Modify(src->Identifier(),events,src.get());
}

void EventLoop::Unregister(int events, std::shared_ptr<Channel> src) {
    poller_->Unregister(src->Identifier(), events);
    channelSet_.erase(src);
}

void EventLoop::Run() {
  const std::chrono::milliseconds defaultPollTime(10);

  while (running_) {
    printf("started base running,%d\n",g_thisLoop);
    _Loop(defaultPollTime);
  }
  for (auto &kv : channelSet_) {
    poller_->Unregister(kv->Identifier(),EPOLL_ET_Read | EPOLL_ET_Write);
  }
  channelSet_.clear();
  poller_.reset();
}

bool EventLoop::_Loop(std::chrono::milliseconds timeout) {
    printf("anyproblem-----%d--------%d\n",channelSet_.size(),g_thisLoop);
    if(channelSet_.empty()) {
        std::this_thread::sleep_for(timeout);
        printf("empty\n");
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
    auto src = static_cast<Channel*>(fired[i].userdata);
    sources[i] = src->shared_from_this();
    if (fired[i].events & EPOLL_ET_Read) {
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
      std::cout << "Error: " << std::endl;
      src->HandleErrorEvent();
    }
  }

  // TODO: process function TRY try_lock
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