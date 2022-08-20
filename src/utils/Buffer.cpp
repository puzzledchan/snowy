/**
 * @file Buffer.cpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-07-30
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "Buffer.hpp"
#include <cassert>
#include <cstring>

inline static std::size_t _RoundUpPower(std::size_t size) {
  if (size == 0)
    return 0;
  std::size_t roundUp = 1;
  while (roundUp < size)
    roundUp *= 2;
  return roundUp;
}

/**
 * @brief 确保buffer存在期望数据长度的空间
 *
 * @param expectedSize 期望的数据长度：已写未读 + 未写
 */
void Buffer::AssureSpace(std::size_t expectedSize) {
  if (writableSize() >= expectedSize) {
    return;
  }
  const size_t dataSize = readableSize();
  const size_t oldCap = capacity_;
  // 调整cap - write + read 即空闲空间
  while (writableSize() + readPos_ < expectedSize) {
    if (capacity_ < Buffer::_DefaultSize) {
      capacity_ = Buffer::_DefaultSize;
    } else if (capacity_ <= Buffer::_MaxBufferSize) {
      const auto newCapacity = _RoundUpPower(capacity_);
      if (capacity_ < newCapacity) {
        capacity_ = newCapacity;
      } else {
        capacity_ = 3 * newCapacity / 2;
      }
    } else
      assert(false); // out of memory
  }
  if (oldCap < capacity_) {
    std::unique_ptr<char[]> temp(new char[capacity_]);
    if (dataSize != 0)
      memcpy(&temp[0], &buffer_[readPos_], dataSize);
    buffer_.swap(temp);
  } else {
    assert(readPos_ > 0);
    ::memmove(&buffer_[0], &buffer_[readPos_], dataSize);
  }
  readPos_ = 0;
  writePos_ = dataSize;
  assert(dataSize == readableSize());
};

std::size_t Buffer::peekDataAt(void *buf, std::size_t size,
                               std::size_t offset) {
  const std::size_t dataSize = readableSize();
  if (!buf || size == 0 || dataSize <= offset)
    return 0;
  if (size + offset > dataSize)
    size = dataSize - offset;
  ::memcpy(buf, &buffer_[readPos_ + offset], size);
  return size;
}

std::size_t Buffer::popData(void *buf, std::size_t size) {
  std::size_t bytes = peekDataAt(buf, size);
  consume(bytes);
  return bytes;
}

std::size_t Buffer::pushData(const void *data, std::size_t size) {
  std::size_t bytes = pushDataAt(data, size);
  produce(bytes);
  assert(bytes == size);
  return bytes;
}

std::size_t Buffer::pushDataAt(const void *data, std::size_t size,
                               std::size_t offset) {
  if (!data || size == 0)
    return 0;
  if (readableSize() + size + offset >= _MaxBufferSize)
    return 0; // overflow
  AssureSpace(size + offset);
  assert(size + offset <= writableSize());
  ::memcpy(&buffer_[writePos_ + offset], data, size);
  return size;
}

void Buffer::consume(std::size_t bytes) {
  assert(readPos_ + bytes <= writePos_);
  readPos_ += bytes;
  if (empty())
    clear();
}
void Buffer::produce(std::size_t bytes) {
  assert(writePos_ + bytes <= capacity_);
  writePos_ += bytes;
}