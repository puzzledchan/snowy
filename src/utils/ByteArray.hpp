/**
 * @file ByteArray.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-08-17
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SNOWY_BYTEARRAY_H
#define SNOWY_BYTEARRAY_H

#include <memory>
#include <vector>
class ByteArray {
public:
  std::vector<char> bytes_;

public:
  ByteArray(size_t base_size = 4094);
  ~ByteArray();
  void clear();
  char *writeAddr() { return &bytes_[write_pos_]; }
  char *readAddr() { return &bytes_[read_pos_]; }
  void produce(std::size_t bytes) { write_pos_ += bytes; }
  void consume(std::size_t bytes) { read_pos_ += bytes; }

public:
  void writeByte(const void *buffer, size_t size);
  template <typename T> void writeFint(T value) {
    writeByte(&value, sizeof(value));
  }

  // 原始写入
  void writeFint8(int8_t value);
  void writeFuint8(uint8_t value);

  void writeFint16(int16_t value);
  void writeFuint16(uint16_t value);

  void writeFint32(int32_t value);
  void writeFuint32(uint32_t value);

  void writeFint64(int64_t value);
  void writeFuint64(uint64_t value);

  // 编码写入
  void writeInt32(int32_t value);
  void writeUint32(uint32_t value);
  void writeInt64(int64_t value);
  void writeUint64(uint64_t value);
  void writeFloat(float value);
  void writeDouble(double value);
  void writeStringF16(const std::string &value);
  void writeStringF32(const std::string &value);
  void writeStringF64(const std::string &value);
  void writeStringVint(const std::string &value);
  void writeStringWithoutLength(const std::string &value);

public:
  void readByte(void *buf, size_t size);
  // 原始读取
  int8_t readFint8();
  uint8_t readFuint8();
  int16_t readFint16();
  uint16_t readFuint16();
  int32_t readFint32();
  uint32_t readFuint32();
  int64_t readFint64();
  uint64_t readFuint64();

  // 解码读取
  int32_t readInt32();
  uint32_t readUint32();
  int64_t readInt64();
  uint64_t readUint64();
  float readFloat();
  double readDouble();
  std::string readStringF16();
  std::string readStringF32();
  std::string readStringF64();
  std::string readStringVint();

  std::string toString() {
    std::string str;
    str.resize(readableSize());
    if (str.empty())
      return str;
    readByte(&str[0], str.size());
    return str;
  }

public:
  size_t read_pos_;
  size_t write_pos_;
  size_t capacity_;

public:
  void assureSpace(size_t size);
  size_t writeableSize() const { return capacity_ - write_pos_; }
  size_t readableSize() const { return write_pos_ - read_pos_; }
};
#endif