/**
 * @file Poller.hpp
 * @author JDongChen
 * @brief IO多路复用
 * @version 0.1
 * @date 2022-08-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef SNOWY_POLLER_H
#define SNOWY_POLLER_H

#include <sys/epoll.h>
#include <unistd.h>

#include <vector>
#include <iostream>

struct FiredEvent {
    int   events;
    void* userdata; // use for channe pointer

    FiredEvent() : events(0), userdata(nullptr) {
    }
};

enum  PollEvent {
    EPOLL_ET_None,
    EPOLL_ET_Read,
    EPOLL_ET_Write,
    EPOLL_ET_ERROR,
};


class Poller {
public:
    Poller() : multiplexer_(-1) {
    }

    virtual ~Poller() {
    }

    virtual bool Register(int fd, int events, void* userPtr) = 0;
    virtual bool Modify(int fd, int events, void* userPtr) = 0;
    virtual bool Unregister(int fd, int events) = 0;

    virtual int Poll(std::size_t maxEv, int timeoutMs) = 0;
    const std::vector<FiredEvent>& GetFiredEvents() const {
        return firedEvents_;
    }

protected:
    int multiplexer_;
    std::vector<FiredEvent>  firedEvents_;
};

class Epoller : public Poller {
public:
    Epoller() {
        multiplexer_ = ::epoll_create(512);
        std::cout << "create epoll:" << multiplexer_ <<std::endl;
    }
    ~Epoller() {
        if(multiplexer_ != -1) {
            std::cout << "close epoll:" << multiplexer_ <<std::endl; 
            ::close(multiplexer_); 
        }
    }

    Epoller(const Epoller& ) = delete;
    void operator= (const Epoller& ) = delete;

public:
    bool Register(int epfd, int events, void* userPtr) override {
      epoll_event ev{0};
      ev.data.ptr = userPtr;
      ev.data.fd = epfd;
      if (events & EPOLL_ET_Read) 
        ev.events |= EPOLLIN;
      if (events & EPOLL_ET_Write)
        ev.events |= EPOLLOUT;

      return 0 == epoll_ctl(multiplexer_, EPOLL_CTL_ADD, epfd, &ev);      
    }
    bool Modify(int epfd, int events, void* userPtr) override {
        epoll_event  ev{0};
        ev.data.ptr= userPtr;
        ev.data.fd = epfd;
        if (events & EPOLL_ET_Read) 
            ev.events |= EPOLLIN;
        if (events & EPOLL_ET_Write)
            ev.events |= EPOLLOUT;

        return 0 == epoll_ctl(multiplexer_, EPOLL_CTL_MOD, epfd, &ev);    
    }
    bool Unregister(int epfd, int events) {
        epoll_event dummy;
        return 0 == epoll_ctl(multiplexer_, EPOLL_CTL_DEL, epfd, &dummy) ;     
    }

    int Poll(std::size_t maxEvent, int timeoutMs) override {
        if(maxEvent == 0) return 0;
        while(events_.size() < maxEvent)        
            events_.resize(2 * events_.size() + 1);
         int nFired = ::epoll_wait(multiplexer_, &events_[0], maxEvent, timeoutMs);
        if (nFired == -1 && errno != EINTR && errno != EWOULDBLOCK)
            return -1;

        //  channel Ptr AND events
        auto& events = firedEvents_;
        if (nFired > 0)
            events.resize(nFired);
        for (int i = 0; i < nFired; ++ i) {
            FiredEvent& fired = events[i];
            fired.events   = 0;
            fired.userdata = events_[i].data.ptr; 

            if (events_[i].events & EPOLLIN)
                fired.events  |= EPOLL_ET_Read;

            if (events_[i].events & EPOLLOUT)
                fired.events  |= EPOLL_ET_Write;

            if (events_[i].events & (EPOLLERR | EPOLLHUP))
                fired.events  |= EPOLL_ET_ERROR;
        }   
        return nFired;
    }

private:
    std::vector<epoll_event> events_;
};


#endif