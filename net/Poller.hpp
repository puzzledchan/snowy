/**
 * @file Poller.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-07-31
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <vector>

struct FiredEvent {
  int events;
  void *userdata;

  FiredEvent() : events(0), userdata(nullptr) {}
};

class Poller {
public:
  enum EventType {
    eET_None = 0,
    eET_Read = 0x1 << 0,
    eET_Write = 0x1 << 1,
    eET_Error = 0x1 << 2,
  };

public:
  Poller() : multiplexer_(-1) {}

  virtual ~Poller() {}

  virtual bool Register(int fd, int events, void *userPtr) = 0;
  virtual bool Modify(int fd, int events, void *userPtr) = 0;
  virtual bool Unregister(int fd, int events) = 0;

  virtual int Poll(std::size_t maxEv, int timeoutMs) = 0;
  const std::vector<FiredEvent> &GetFiredEvents() const { return firedEvents_; }

protected:
  int multiplexer_;
  std::vector<FiredEvent> firedEvents_;
};
