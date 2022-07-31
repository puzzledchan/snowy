/**
 * @file Epoller.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-07-31
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "sys/epoll.h"
#include <vector>

#include "Poller.hpp"

class Epoller : public Poller {
public:
  Epoller();
  ~Epoller();

  Epoller(const Epoller &) = delete;
  void operator=(const Epoller &) = delete;

  bool Register(int fd, int events, void *userPtr) override;
  bool Modify(int fd, int events, void *userPtr) override;
  bool Unregister(int fd, int events) override;

  int Poll(std::size_t maxEvent, int timeoutMs) override;

private:
  std::vector<epoll_event> events_;
};