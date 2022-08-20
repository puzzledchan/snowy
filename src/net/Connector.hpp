/**
 * @file Connector.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-08-20
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <functional>

#include "Channel.hpp"
enum class ConnectState {
  None,
  Connecting,
  Connected,
  Failed,
};
class EventLoop;

class Connector : public Channel {
  std::shared_ptr<EventLoop> loop_;
  int local_sock_;
  uint16_t local_port_;
  sockaddr_in peer_;
  ConnectState state_ = ConnectState::None;

  static const int kInvaild_ = -1;
  static const uint16_t kInvalidPort_ = -1;
  using MakeNewConnection =
      std::function<void(int connfd, const sockaddr_in &peer)>;
  MakeNewConnection makeNewConnection;

public:
  explicit Connector(std::shared_ptr<EventLoop> loop) {
    loop_ = loop;
    local_sock_ = kInvaild_;
    local_port_ = kInvalidPort_;
  }
  Connector(const Connector &) = delete;
  void operator=(const Connector &) = delete;

public:
  int Identifier() const override;
  bool HandleReadEvent() override;
  bool HandleWriteEvent() override;
  void HandleErrorEvent() override;
  void setMakeNewConnection(MakeNewConnection func) {
    makeNewConnection = func;
  }
};