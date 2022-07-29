/**
 * @file Connection.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-08-20
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SNOWY_CONNECTION_H
#define SNOWY_CONNECTION_H
#include <arpa/inet.h>

#include "Buffer.hpp"
#include "Channel.hpp"
#include "EventLoop.hpp"

class Connection : public Channel {
private:
  enum State {
    None,
    Connected,
    CloseWaitWrite,
    PassiveClose,
    ActiveClose,
    Error,
    Closed,
  };
  enum ShutdownMode {
    SM_BOTH,
    SM_READ,
    SM_WRITE,
  };

protected:
  std::shared_ptr<EventLoop> loop_;
  int local_sock_;
  sockaddr_in peer_;
  static const int kInvalid_ = -1;
  State state_ = None;

protected:
  Buffer recv_buf_;
  Buffer send_buf_;

public:
  explicit Connection(std::shared_ptr<EventLoop> loop);
  //~Connection();
  Connection(const Connection &) = delete;
  void operator=(const Connection &) = delete;
  bool Init(int sock, const sockaddr_in &peer);

public:
  int Identifier() const override;
  bool HandleReadEvent() override;
  bool HandleWriteEvent() override;
  void HandleErrorEvent() override;
  virtual void processMessage();

protected:
  void _Shutdown(ShutdownMode mode);
};
#endif