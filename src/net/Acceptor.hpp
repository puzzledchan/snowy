
#include <arpa/inet.h>

#include <atomic>
#include <vector>

#include <cassert>

#include "Channel.hpp"


//class Connection;
class EventLoop;
// TODO RESOURCE MANAGER
class Acceptor: public Channel {
private:
    std::shared_ptr<EventLoop> baseloop_;
    std::vector<std::shared_ptr<EventLoop>> loop_pool_;
    std::atomic<int> current_loop_ind_;

    int local_sock_;
    uint16_t local_port_;
    sockaddr_in peer_;

    static const int kInvaild_ = -1;
    static const uint16_t kInvalidPort_ = -1;

public:
    explicit Acceptor( std::shared_ptr<EventLoop> loop,
     std::vector<std::shared_ptr<EventLoop>> loop_pool) {
        baseloop_ = loop;
        loop_pool_ = loop_pool;
        local_sock_ = kInvaild_;
        local_port_ = kInvalidPort_;
        current_loop_ind_.store(0);
     }
    void BindAndListen();
public:
    int Identifier() const override {return local_sock_;}
    bool HandleReadEvent() override ;
    bool HandleWriteEvent() override ;
    void HandleErrorEvent() override ;

private:
  int _Accept() {
    socklen_t addrlen = sizeof(peer_);
    return ::accept(local_sock_,(struct sockaddr *)&peer_,&addrlen);
  }

};