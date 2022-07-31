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
  std::vector<char> buffer_;

  static const std::size_t _MaxBufferSize =
      std::numeric_limits<std::size_t>::max() / 2;
  static const std::size_t _HighWaterMark = 1 * 1023;
  static const std::size_t _DefaultSize = 256;

public:
  Buffer() : readPos_(0), writePos_(0), capacity_(0) {}

  std::size_t ReadableSize() const { return writePos_ - readPos_; }
  std::size_t WritableSize() const { return capacity_ - writePos_; }
  std::size_t Capacity() const { return capacity_; }
  char *ReadArr() { return &buffer_[readPos_]; }
  char *WriteArr() { return &buffer_[writePos_]; }
  char *BufferAddr() { return &*buffer_.rbegin(); }

public:
  std::size_t Append(const void *data, std::size_t size);
  void AssureSpace(std::size_t size);
};

#endif