/**
 * @file Buffer.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-07-30
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SNOWY_BUFFER_H
#define SNOWY_BUFFER_H

#include <limits>
#include <memory>
#include <vector>
class Buffer {
private:
  std::size_t readPos_;
  std::size_t writePos_;
  std::size_t capacity_;
  std::unique_ptr<char[]> buffer_;

  static const std::size_t _MaxBufferSize =
      std::numeric_limits<std::size_t>::max() / 2;
  static const std::size_t _DefaultSize = 1024;

public:
  Buffer() : readPos_(0), writePos_(0) {
    capacity_ = _DefaultSize;
    buffer_.reset(new char[capacity_]);
  }

  std::size_t readableSize() const { return writePos_ - readPos_; }
  std::size_t writableSize() const { return capacity_ - writePos_; }
  std::size_t capacity() const { return capacity_; }
  char *readAddr() { return &buffer_[readPos_]; }
  char *writeAddr() { return &buffer_[writePos_]; }

  bool empty() { return readPos_ == writePos_; }
  void clear() { readPos_ = writePos_ = 0; }
  void produce(std::size_t bytes);
  void consume(std::size_t bytes);

public:
  // read from buffer  retrive or not
  std::size_t popData(void *buf, std::size_t size);
  std::size_t peekDataAt(void *buf, std::size_t size, std::size_t offset = 0);
  // write to data
  std::size_t pushData(const void *buf, std::size_t size);
  std::size_t pushDataAt(const void *data, std::size_t size,
                         std::size_t offset = 0);

public:
  void AssureSpace(std::size_t size);
};

#endif