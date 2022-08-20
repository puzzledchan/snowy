/**
 * @file serializer.hpp
 * @author JDongChen
 * @brief RPC 序列化 / 反序列化包装，会自动进行网络序转换
 * @details 序列化有以下规则：
 * 1.默认情况下序列化，8，16位类型以及浮点数不压缩，32，64位有符号/无符号数采用
 * zigzag 和 varints 编码压缩 2.针对 std::string
 * 会将长度信息压缩序列化作为元数据，然后将原数据直接写入。char数组会先转换成
 * std::string 后按此规则序列化 3.调用 writeFint 将不会压缩数字，调用
 * writeRowData 不会加入长度信息
 *
 * 支持标准库容器：
 * 顺序容器：string, list, vector
 * 关联容器：set, multiset, map, multimap
 * 无序容器：unordered_set, unordered_multiset, unordered_map,
 * unordered_multimap 异构容器：tuple
 * @version 0.1
 * @date 2022-08-14
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SNOWY_SERIALIZER_H
#define SNOWY_SERIALIZER_H

#include <list>
#include <map>
#include <set>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ByteArray.hpp"
class Serializer {
public:
  Serializer() { byte_array_ = std::make_shared<ByteArray>(); }
  Serializer(std::shared_ptr<ByteArray> byteArray) { byte_array_ = byteArray; }
  Serializer(const std::string &in) {
    byte_array_ = std::make_shared<ByteArray>();
    writeRowData(&in[0], in.size());
  }

public:
  // int size() { return byte_array_->getSize(); }
  // void reset() { byte_array_->setPosition(0); }
  std::string toString() { return byte_array_->toString(); }
  /**
   * @brief 写入原始数据
   */
  void writeRowData(const char *in, int len) {
    byte_array_->writeByte(in, len);
  }
  /**
   * @brief 写入无压缩数字
   */
  template <class T> void writeFint(T value) { byte_array_->writeFint(value); }

  void clear() { byte_array_->clear(); }

  template <typename T> void readc(T &t) {
    if constexpr (std::is_same_v<T, bool>) {
      t = byte_array_->readFint8();
    } else if constexpr (std::is_same_v<T, float>) {
      t = byte_array_->readFloat();
    } else if constexpr (std::is_same_v<T, double>) {
      t = byte_array_->readDouble();
    } else if constexpr (std::is_same_v<T, int8_t>) {
      t = byte_array_->readFint8();
    } else if constexpr (std::is_same_v<T, uint8_t>) {
      t = byte_array_->readFuint8();
    } else if constexpr (std::is_same_v<T, int16_t>) {
      t = byte_array_->readFint16();
    } else if constexpr (std::is_same_v<T, uint16_t>) {
      t = byte_array_->readFuint16();
    } else if constexpr (std::is_same_v<T, int32_t>) {
      t = byte_array_->readInt32();
    } else if constexpr (std::is_same_v<T, uint32_t>) {
      t = byte_array_->readUint32();
    } else if constexpr (std::is_same_v<T, int64_t>) {
      t = byte_array_->readInt64();
    } else if constexpr (std::is_same_v<T, uint64_t>) {
      t = byte_array_->readUint64();
    } else if constexpr (std::is_same_v<T, std::string>) {
      t = byte_array_->readStringVint();
    }
  }

  template <typename T> void writec(T t) {
    if constexpr (std::is_same_v<T, bool>) {
      byte_array_->writeFint8(t);
    } else if constexpr (std::is_same_v<T, float>) {
      byte_array_->writeFloat(t);
    } else if constexpr (std::is_same_v<T, double>) {
      byte_array_->writeDouble(t);
    } else if constexpr (std::is_same_v<T, int8_t>) {
      byte_array_->writeFint8(t);
    } else if constexpr (std::is_same_v<T, uint8_t>) {
      byte_array_->writeFuint8(t);
    } else if constexpr (std::is_same_v<T, int16_t>) {
      byte_array_->writeFint16(t);
    } else if constexpr (std::is_same_v<T, uint16_t>) {
      byte_array_->writeFuint16(t);
    } else if constexpr (std::is_same_v<T, int32_t>) {
      byte_array_->writeInt32(t);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
      byte_array_->writeUint32(t);
    } else if constexpr (std::is_same_v<T, int64_t>) {
      byte_array_->writeInt64(t);
    } else if constexpr (std::is_same_v<T, uint64_t>) {
      byte_array_->writeUint64(t);
    } else if constexpr (std::is_same_v<T, std::string>) {
      byte_array_->writeStringVint(t);
    } else if constexpr (std::is_same_v<T, char *>) {
      byte_array_->writeStringVint(std::string(t));
    } else if constexpr (std::is_same_v<T, const char *>) {
      byte_array_->writeStringVint(std::string(t));
    }
  }

public:
  template <typename T>[[maybe_unused]] Serializer &operator>>(T &i) {
    readc(i);
    return *this;
  }

  template <typename T>[[maybe_unused]] Serializer &operator<<(const T &i) {
    writec(i);
    return *this;
  }
  template <typename... Args> Serializer &operator>>(std::tuple<Args...> &t) {
    /**
     * @brief 实际的反序列化函数，利用折叠表达式展开参数包
     */
    const auto &deserializer = [this]<typename Tuple, std::size_t... Index>(
        Tuple & t, std::index_sequence<Index...>) {
      (void)((*this) >> ... >> std::get<Index>(t));
    };
    deserializer(t, std::index_sequence_for<Args...>{});
    return *this;
  }

  template <typename... Args>
  Serializer &operator<<(const std::tuple<Args...> &t) {
    /**
     * @brief 实际的序列化函数，利用折叠表达式展开参数包
     */
    const auto &package = [this]<typename Tuple, std::size_t... Index>(
        const Tuple &t, std::index_sequence<Index...>) {
      (void)((*this) << ... << std::get<Index>(t));
    };
    package(t, std::index_sequence_for<Args...>{});
    return *this;
  }

  template <typename T> Serializer &operator>>(std::list<T> &v) {
    size_t size;
    readc(size);
    for (size_t i = 0; i < size; ++i) {
      T t;
      readc(t);
      v.template emplace_back(t);
    }
    return *this;
  }

  template <typename T> Serializer &operator<<(const std::list<T> &v) {
    writec(v.size());
    for (auto &t : v) {
      (*this) << t;
    }
    return *this;
  }

  template <typename T> Serializer &operator>>(std::vector<T> &v) {
    size_t size;
    readc(size);
    for (size_t i = 0; i < size; ++i) {
      T t;
      readc(t);
      v.template emplace_back(t);
    }
    return *this;
  }

  template <typename T> Serializer &operator<<(const std::vector<T> &v) {
    writec(v.size());
    for (auto &t : v) {
      (*this) << t;
    }
    return *this;
  }

  template <typename T> Serializer &operator>>(std::set<T> &v) {
    size_t size;
    readc(size);
    for (size_t i = 0; i < size; ++i) {
      T t;
      readc(t);
      v.template emplace(t);
    }
    return *this;
  }

  template <typename T> Serializer &operator<<(const std::set<T> &v) {
    writec(v.size());
    for (auto &t : v) {
      (*this) << t;
    }
    return *this;
  }

  template <typename T> Serializer &operator>>(std::multiset<T> &v) {
    size_t size;
    readc(size);
    for (size_t i = 0; i < size; ++i) {
      T t;
      readc(t);
      v.template emplace(t);
    }
    return *this;
  }

  template <typename T> Serializer &operator<<(const std::multiset<T> &v) {
    writec(v.size());
    for (auto &t : v) {
      (*this) << t;
    }
    return *this;
  }

  template <typename T> Serializer &operator>>(std::unordered_set<T> &v) {
    size_t size;
    readc(size);
    for (size_t i = 0; i < size; ++i) {
      T t;
      readc(t);
      v.template emplace(t);
    }
    return *this;
  }

  template <typename T> Serializer &operator<<(const std::unordered_set<T> &v) {
    writec(v.size());
    for (auto &t : v) {
      (*this) << t;
    }
    return *this;
  }

  template <typename T> Serializer &operator>>(std::unordered_multiset<T> &v) {
    size_t size;
    readc(size);
    for (size_t i = 0; i < size; ++i) {
      T t;
      readc(t);
      v.template emplace(t);
    }
    return *this;
  }

  template <typename T>
  Serializer &operator<<(const std::unordered_multiset<T> &v) {
    writec(v.size());
    for (auto &t : v) {
      (*this) << t;
    }
    return *this;
  }

  template <typename K, typename V>
  Serializer &operator<<(const std::pair<K, V> &m) {
    (*this) << m.first << m.second;
    return *this;
  }

  template <typename K, typename V> Serializer &operator>>(std::pair<K, V> &m) {
    (*this) >> m.first >> m.second;
    return *this;
  }

  template <typename K, typename V> Serializer &operator>>(std::map<K, V> &m) {
    size_t size;
    readc(size);
    for (size_t i = 0; i < size; ++i) {
      std::pair<K, V> p;
      (*this) >> p;
      m.template emplace(p);
    }
    return *this;
  }

  template <typename K, typename V>
  Serializer &operator<<(const std::map<K, V> &m) {
    writec(m.size());
    for (auto &t : m) {
      (*this) << t;
    }
    return *this;
  }

  template <typename K, typename V>
  Serializer &operator>>(std::unordered_map<K, V> &m) {
    size_t size;
    readc(size);
    for (size_t i = 0; i < size; ++i) {
      std::pair<K, V> p;
      (*this) >> p;
      m.template emplace(p);
    }
    return *this;
  }

  template <typename K, typename V>
  Serializer &operator<<(const std::unordered_map<K, V> &m) {
    writec(m.size());
    for (auto &t : m) {
      (*this) << t;
    }
    return *this;
  }

  template <typename K, typename V>
  Serializer &operator>>(std::multimap<K, V> &m) {
    size_t size;
    readc(size);
    for (size_t i = 0; i < size; ++i) {
      std::pair<K, V> p;
      (*this) >> p;
      m.template emplace(p);
    }
    return *this;
  }

  template <typename K, typename V>
  Serializer &operator<<(const std::multimap<K, V> &m) {
    writec(m.size());
    for (auto &t : m) {
      (*this) << t;
    }
    return *this;
  }

  template <typename K, typename V>
  Serializer &operator>>(std::unordered_multimap<K, V> &m) {
    size_t size;
    readc(size);
    for (size_t i = 0; i < size; ++i) {
      std::pair<K, V> p;
      (*this) >> p;
      m.template emplace(p);
    }
    return *this;
  }

  template <typename K, typename V>
  Serializer &operator<<(const std::unordered_multimap<K, V> &m) {
    writec(m.size());
    for (auto &t : m) {
      (*this) << t;
    }
    return *this;
  }

public:
  std::shared_ptr<ByteArray> byte_array_;
};

#endif