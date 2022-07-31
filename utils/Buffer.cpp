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

inline static std::size_t RoundUpPower(std::size_t size) {
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
  if (WritableSize() >= expectedSize) {
    return;
  }
  const size_t dataSize = ReadableSize();
  const size_t oldCap = capacity_;
  // 调整cap - write + read 即空闲空间
  while (WritableSize() + readPos_ < expectedSize) {
    if (capacity_ < Buffer::_DefaultSize) {
      capacity_ = Buffer::_DefaultSize;
    } else if (capacity_ <= Buffer::_MaxBufferSize) {
      const auto newCapacity = RoundUpPower(capacity_);
      if (capacity_ < newCapacity) {
        capacity_ = newCapacity;
      } else {
        capacity_ = 3 * newCapacity / 2;
      }
    } else
      assert(false);
  }
  if (oldCap < capacity_) {
    buffer_.resize(capacity_);
    std::copy(buffer_.begin() + readPos_, buffer_.begin() + writePos_,
              buffer_.begin());
  } else {
    std::copy(buffer_.begin() + readPos_, buffer_.begin() + writePos_,
              buffer_.begin());
  }
  readPos_ = 0;
  writePos_ = dataSize;
  assert(dataSize == ReadableSize());
};

std::size_t Buffer::Append(const void *data, std::size_t size) {
  if (!data || size == 0)
    return 0;

  AssureSpace(size);
  char *ptr = (char *)data;
  std::copy(ptr, ptr + size, WriteArr());
  writePos_ += size;
  return size;
}