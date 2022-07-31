/**
 * @file Channel.h
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-07-31
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SNOWY_CHANNEL_H
#define SNOWY_CHANNEL_H
#include <iostream>
#include <memory>
class Channel : public std::enable_shared_from_this<Channel> {

public:
  Channel() { std::cout << "New Channel" << std::endl; }
  virtual ~Channel() { std::cout << "Delete Channel" << std::endl; };
  Channel(const Channel &) = delete;
  void operator=(const Channel &) = delete;

  virtual int Identifier() const = 0;
  virtual bool HandleReadEvent() = 0;
  virtual bool HandleWriteEvent() = 0;
  virtual void HandleErrorEvent() = 0;
  ///@brief The unique id, it'll not repeat in whole process.
  unsigned int GetUniqueId() const { return unique_id_; }

  ///@brief Set the unique id, it's called by library.
  void SetUniqueId(unsigned int id) { unique_id_ = id; }

private:
  unsigned int unique_id_;
};
#endif //