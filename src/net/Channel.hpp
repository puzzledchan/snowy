/**
 * @file Channel.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-08-06
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SNOWY_CHANNEL_H
#define SNOWY_CHANNEL_H

#include "Socket.hpp"
#include <memory>
#include <stdio.h>

class Channel : public std::enable_shared_from_this<Channel> {
public:
  Channel() { printf("New channel %p\n", (void *)this); }
  virtual ~Channel() { printf("Delete channel %p\n", (void *)this); }

  Channel(const Channel &) = delete;
  void operator=(const Channel &) = delete;

  ///@brief Return socket fd for sockets, just for debug
  virtual int Identifier() const = 0;
  ///@brief When read event occurs
  virtual bool HandleReadEvent() = 0;
  ///@brief When write event occurs
  virtual bool HandleWriteEvent() = 0;
  ///@brief When error event occurs
  virtual void HandleErrorEvent() = 0;
};

#endif