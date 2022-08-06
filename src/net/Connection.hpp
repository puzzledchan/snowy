#include <arpa/inet.h>

#include "Channel.hpp"


class EventLoop;
class Connection:public Channel {
private:
    enum State {
        None,
        Connected,
        CloseWaitWrite,
        PassiveClose,
        ActiveClose,
        Closed,
    };
private:
std::shared_ptr<EventLoop> loop_;
int local_sock_;
sockaddr_in peer_;

static const int kInvalid_ = -1;

public:
    Connection(std::shared_ptr<EventLoop> loop);
    ~Connection();
    Connection(const Connection& ) = delete;
    void operator= (const Connection& ) = delete;

/*     bool Init(int fd, const sockaddr_in&peer);
    const sockaddr_in& getPeer() {
        return peer_;
    }
    std::shared_ptr<EventLoop> GetLoop() const {
        return loop_.lock();
    } */

public:
    int Identifier() const override;
    bool HandleReadEvent() override;
    bool HandleWriteEvent() override;
    void HandleErrorEvent() override;
};