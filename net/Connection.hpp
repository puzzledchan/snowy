/**
 * @file Connection.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-07-31
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SNOWY_CONNECTION_H
#define SNOWY_CONNECTION_H
#include <Channel.hpp>

class EventLoop;
class Connection : public Channel {
private:
  enum State {
    None,
    Connected,
    CloseWaitWrite,
    PassiveClose,
    ActiveClose,
    Error,
    Close,
  };

private:
  int connfd_;
  EventLoop *const loop_;

public:
  explicit Connection(EventLoop *loop);
  ~Connection();

  Connection(const Connection &) = delete;
  void operator=(const Connection &) = delete;

  ///@brief Return socket fd
  int Identifier() const override;

  ///@brief Process ReadEvents
  bool HandleReadEvent() override;
  ///@brief Process WriteEvents
  bool HandleWriteEvent() override;
  ///@brief Process ErrorEvents
  void HandleErrorEvent() override;
};
#endif /* SNOWY_CONNECTION_H */