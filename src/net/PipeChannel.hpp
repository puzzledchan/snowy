/**
 * @file PipeChannel.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-08-20
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SNOWY_PIPECHANNEL_H
#define SNOWY_PIPECHANNEL_H
#include <cassert>

#include "Channel.hpp"

class PipeChannel : public Channel {
public:
  PipeChannel() {
    int fd[2];
    int ret = ::pipe(fd);
    assert(ret == 0);
    readFd_ = fd[0];
    writeFd_ = fd[1];
    SetNonBlock(readFd_, true);
    SetNonBlock(writeFd_, true);
  }
  ~PipeChannel() {
    ::close(readFd_);
    ::close(writeFd_);
  }

  PipeChannel(const PipeChannel &) = delete;
  void operator=(const PipeChannel &) = delete;

  int Identifier() const override { return readFd_; }
  bool HandleReadEvent() override {
    char ch;
    auto n = ::read(readFd_, &ch, sizeof(ch));
    return n == 1;
  }
  bool HandleWriteEvent() override {
    assert(false);
    return false;
  }
  void HandleErrorEvent() override {}

  bool Notify() {
    char ch = 0;
    auto n = ::write(writeFd_, &ch, sizeof(ch));
    return n == 1;
  }

private:
  int readFd_;
  int writeFd_;
};

#endif