
#include "Poller.hpp"

Epoller::Epoller() {
  multiplexer_ = ::epoll_create1(EPOLL_CLOEXEC);
  printf("create Epoller: %d\n", multiplexer_);
}

Epoller::~Epoller() {
  if (multiplexer_ != -1) {
    printf("close Epoller: %d\n", multiplexer_);
    ::close(multiplexer_);
  };
}

bool Epoller::Register(int epfd, int events, void *userPtr) {
  epoll_event ev{0};
  ev.data.ptr = userPtr;
  if (events & EPOLL_ET_Read) {
    ev.events |= EPOLLIN;
    ev.events |= EPOLLET;
  }

  if (events & EPOLL_ET_Write)
    ev.events |= EPOLLOUT;

  return 0 == epoll_ctl(multiplexer_, EPOLL_CTL_ADD, epfd, &ev);
}

bool Epoller::Modify(int epfd, int events, void *userPtr) {
  epoll_event ev{0};
  ev.data.ptr = userPtr;
  if (events & EPOLL_ET_Read)
    ev.events |= EPOLLIN;
  if (events & EPOLL_ET_Write)
    ev.events |= EPOLLOUT;

  return 0 == epoll_ctl(multiplexer_, EPOLL_CTL_MOD, epfd, &ev);
}
bool Epoller::Unregister(int epfd, int events) {
  epoll_event dummy;
  return 0 == epoll_ctl(multiplexer_, EPOLL_CTL_DEL, epfd, &dummy);
}

int Epoller::Poll(std::size_t maxEvent, int timeoutMs) {
  if (maxEvent == 0)
    return 0;
  while (events_.size() < maxEvent)
    events_.resize(2 * events_.size() + 1);
  int nFired = ::epoll_wait(multiplexer_, &events_[0], maxEvent, timeoutMs);
  if (nFired == -1 && errno != EINTR && errno != EWOULDBLOCK)
    return -1;
  //  received events set channel
  if (nFired > 0)
    firedEvents_.resize(nFired);
  for (int i = 0; i < nFired; ++i) {
    FiredEvent &fired = firedEvents_[i];
    fired.events = 0;
    fired.userdata = events_[i].data.ptr;
    if (events_[i].events & EPOLLIN)
      fired.events |= EPOLL_ET_Read;

    if (events_[i].events & EPOLLOUT)
      fired.events |= EPOLL_ET_Write;

    if (events_[i].events & (EPOLLERR | EPOLLHUP))
      fired.events |= EPOLL_ET_ERROR;
  }
  return nFired;
}
