#include <byteswap.h>
#include <string.h>

#include <cmath>

#include "ByteArray.hpp"

/**
 * @brief 字节序转换
 */
template <std::integral T> constexpr T ByteSwap(T value) {
  if constexpr (sizeof(T) == sizeof(uint8_t)) {
    return (T)bswap_16((uint16_t)value);
  } else if constexpr (sizeof(T) == sizeof(uint16_t)) {
    return (T)bswap_16((uint16_t)value);
  } else if constexpr (sizeof(T) == sizeof(uint32_t)) {
    return (T)bswap_32((uint32_t)value);
  } else if constexpr (sizeof(T) == sizeof(uint64_t)) {
    return (T)bswap_64((uint64_t)value);
  }
}

ByteArray::ByteArray(size_t base_size) : capacity_(base_size) {
  read_pos_ = write_pos_ = 0;
  bytes_.reserve(capacity_);
}

ByteArray::~ByteArray() {}

void ByteArray::writeFint8(int8_t value) { writeByte(&value, sizeof(value)); }
void ByteArray::writeFuint8(uint8_t value) { writeByte(&value, sizeof(value)); }
void ByteArray::writeFint16(int16_t value) { writeFint(value); }
void ByteArray::writeFuint16(uint16_t value) { writeFint(value); }
void ByteArray::writeFint32(int32_t value) { writeFint(value); }
void ByteArray::writeFuint32(uint32_t value) { writeFint(value); }
void ByteArray::writeFint64(int64_t value) { writeFint(value); }
void ByteArray::writeFuint64(uint64_t value) { writeFint(value); }

// 编码
static uint32_t EncodeZigzag32(const int32_t &v) {
  if (v < 0) {
    return ((uint32_t)(-v)) * 2 - 1;
  } else {
    return v * 2;
  }
}

static uint64_t EncodeZigzag64(const int64_t &v) {
  if (v < 0) {
    return ((uint64_t)(-v)) * 2 - 1;
  } else {
    return v * 2;
  }
}

static int32_t DecodeZigzag32(const uint32_t &v) { return (v >> 1) ^ -(v & 1); }

static int64_t DecodeZigzag64(const uint64_t &v) { return (v >> 1) ^ -(v & 1); }

void ByteArray::writeInt32(int32_t value) {
  writeUint32(EncodeZigzag32(value));
}
void ByteArray::writeUint32(uint32_t value) {
  uint8_t tmp[5];
  uint8_t i = 0;
  while (value >= 0x80) {
    tmp[i++] = (value & 0x7F) | 0x80;
    value >>= 7;
  }
  tmp[i++] = value;
  writeByte(tmp, i);
}

void ByteArray::writeInt64(int64_t value) {
  writeUint64(EncodeZigzag64(value));
}
void ByteArray::writeUint64(uint64_t value) {
  uint8_t tmp[10];
  uint8_t i = 0;
  while (value >= 0x80) {
    tmp[i++] = (value & 0x7F) | 0x80;
    value >>= 7;
  }
  tmp[i++] = value;
  writeByte(tmp, i);
}

void ByteArray::writeFloat(float value) {
  uint32_t v;
  memcpy(&v, &value, sizeof(value));
  writeFuint32(v);
}

void ByteArray::writeDouble(double value) {
  uint64_t v;
  memcpy(&v, &value, sizeof(value));
  writeFuint64(v);
}

// ==============WRITE STRING============================== //
void ByteArray::writeStringF16(const std::string &value) {
  writeFuint16(value.size());
  writeByte(value.c_str(), value.size());
}

void ByteArray::writeStringF32(const std::string &value) {
  writeFuint32(value.size());
  writeByte(value.c_str(), value.size());
}

void ByteArray::writeStringF64(const std::string &value) {
  writeFuint64(value.size());
  writeByte(value.c_str(), value.size());
}

void ByteArray::writeStringVint(const std::string &value) {
  writeUint64(value.size());
  writeByte(value.c_str(), value.size());
}

void ByteArray::writeStringWithoutLength(const std::string &value) {
  writeByte(value.c_str(), value.size());
}

int8_t ByteArray::readFint8() {
  int8_t v;
  readByte(&v, sizeof(v));
  return v;
}

uint8_t ByteArray::readFuint8() {
  uint8_t v;
  readByte(&v, sizeof(v));
  return v;
}

int16_t ByteArray::readFint16() {
  int16_t value;
  readByte(&value, sizeof(value));
  return value;
}
uint16_t ByteArray::readFuint16() {
  uint16_t value;
  readByte(&value, sizeof(value));
  return value;
}

int32_t ByteArray::readFint32() {
  int32_t value;
  readByte(&value, sizeof(value));
  return value;
}

uint32_t ByteArray::readFuint32() {
  uint32_t value;
  readByte(&value, sizeof(value));
  return value;
}

int64_t ByteArray::readFint64() {
  int64_t value;
  readByte(&value, sizeof(value));
  return value;
}

uint64_t ByteArray::readFuint64() {
  uint64_t value;
  readByte(&value, sizeof(value));
  return value;
}

int32_t ByteArray::readInt32() { return DecodeZigzag32(readUint32()); }

uint32_t ByteArray::readUint32() {
  uint32_t result = 0;
  for (int i = 0; i < 32; i += 7) {
    uint8_t b = readFuint8();
    if (b < 0x80) {
      result |= ((uint32_t)b) << i;
      break;
    } else {
      result |= (((uint32_t)(b & 0x7f)) << i);
    }
  }
  return result;
}

int64_t ByteArray::readInt64() { return DecodeZigzag64(readUint64()); }

uint64_t ByteArray::readUint64() {
  uint64_t result = 0;
  for (int i = 0; i < 64; i += 7) {
    uint8_t b = readFuint8();
    if (b < 0x80) {
      result |= ((uint64_t)b) << i;
      break;
    } else {
      result |= (((uint64_t)(b & 0x7f)) << i);
    }
  }
  return result;
}

float ByteArray::readFloat() {
  uint32_t v = readFuint32();
  float value;
  memcpy(&value, &v, sizeof(v));
  return value;
}

double ByteArray::readDouble() {
  uint64_t v = readFuint64();
  double value;
  memcpy(&value, &v, sizeof(v));
  return value;
}

std::string ByteArray::readStringF16() {
  uint16_t len = readFuint16();
  std::string buff;
  buff.resize(len);
  readByte(&buff[0], len);
  return buff;
}

std::string ByteArray::readStringF32() {
  uint32_t len = readFuint32();
  std::string buff;
  buff.resize(len);
  readByte(&buff[0], len);
  return buff;
}

std::string ByteArray::readStringF64() {
  uint64_t len = readFuint64();
  std::string buff;
  buff.resize(len);
  readByte(&buff[0], len);
  return buff;
}

std::string ByteArray::readStringVint() {
  uint64_t len = readUint64();
  std::string buff;
  buff.resize(len);
  readByte(&buff[0], len);
  return buff;
}

void ByteArray::clear() { bytes_.clear(); }

void ByteArray::writeByte(const void *buf, size_t size) {
  if (size == 0)
    return;
  assureSpace(size);
  memcpy(&bytes_[write_pos_], buf, size);
  write_pos_ += size;
}

void ByteArray::readByte(void *buf, size_t size) {
  if (size > readableSize()) {
    throw std::out_of_range("not enough len");
  }
  memcpy(buf, &bytes_[read_pos_], size);
  read_pos_ += size;
}

void ByteArray::assureSpace(size_t size) {
  if (size == 0) {
    return;
  }
  const size_t oldCap = capacity_;
  const size_t dataSize = readableSize();
  while (writeableSize() + read_pos_ < size) {
    capacity_ *= 2;
  }
  if (oldCap < capacity_) {
    std::vector<char> temp;
    temp.resize(capacity_);

    if (dataSize != 0)
      std::copy(bytes_.begin() + read_pos_, bytes_.begin() + write_pos_,
                temp.begin());
    bytes_.swap(temp);
  } else {
    std::copy(bytes_.begin() + read_pos_, bytes_.begin() + write_pos_,
              bytes_.begin());
  }

  read_pos_ = 0;
  write_pos_ = dataSize;
}