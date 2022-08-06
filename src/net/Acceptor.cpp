#include "fcntl.h"

#include "Acceptor.hpp"
#include "EventLoop.hpp"



void Acceptor::BindAndListen() {
    struct sockaddr_in addr;

    local_port_ = 4250;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(local_port_);

    local_sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if(local_sock_ < 0) {
        printf("failed create TCP listen\n");
    } else {
        printf("create TCP listen success\n");
    }
    int ret = ::bind(local_sock_, (struct sockaddr *)&addr, sizeof(addr));
    ret = ::listen(local_sock_,1024);
    fcntl(local_sock_, F_SETFL, fcntl(local_sock_, F_GETFD, 0) | O_NONBLOCK);
}


bool Acceptor::HandleReadEvent() {
    while(true) {
        // ACCEPT  
        // TODO CHOOSE ONE LOOP
        // TODO MAKE CONNECTION
        // TODO LOOP REGISTER AS A FUNCTION
        int connfd = _Accept();
        if(connfd!= kInvaild_) {
         std::cout << "Make Connection successful" << std::endl;
            if(loop_pool_.size()) {
            auto loop = loop_pool_[++current_loop_ind_%loop_pool_.size()];
            auto func = [loop, connfd, peer = peer_] {
 /*                std::shared_ptr<Channel> conn = std::make_shared<Connection>(loop);
                if(loop->Register(EPOLL_ET_Read,conn)) {
                    std::cout << "Make Connection" <<std::endl;
                } */
            };
            loop->RunInThisLoop(std::move(func));
            }

        } else {
            std::cout << "Make Connection failed" << std::endl;
        }

    }
}


bool Acceptor::HandleWriteEvent() {
    assert(false);
    return false;
}

void Acceptor::HandleErrorEvent() {
    assert(false);
    return ;
}