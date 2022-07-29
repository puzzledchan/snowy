
#include <arpa/inet.h>

#include <atomic>
#include <vector>

#include <cassert>

#include "Channel.hpp"
#include "Connection.hpp"

class EventLoop;
// TODO RESOURCE MANAGER
class Acceptor : public Channel {
private:
  std::shared_ptr<EventLoop> loop_;
  std::atomic<int> current_loop_ind_;

  int local_sock_;
  uint16_t local_port_;
  sockaddr_in peer_;

  static const int kInvaild_ = -1;
  static const uint16_t kInvalidPort_ = -1;

  using MakeNewConnection =
      std::function<void(int connfd, const sockaddr_in &peer)>;
  MakeNewConnection makeNewConnection;

public:
  explicit Acceptor(std::shared_ptr<EventLoop> loop) {
    loop_ = loop;
    local_sock_ = kInvaild_;
    local_port_ = kInvalidPort_;
    current_loop_ind_.store(0);
  }
  void BindAndListen();

public:
  int Identifier() const override;
  bool HandleReadEvent() override;
  bool HandleWriteEvent() override;
  void HandleErrorEvent() override;

  void setMakeNewConnection(MakeNewConnection func) {
    makeNewConnection = func;
  }

private:
  int _Accept() {
    socklen_t addrlen = sizeof(peer_);
    return ::accept(local_sock_, (struct sockaddr *)&peer_, &addrlen);
  }
};