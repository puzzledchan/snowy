/**
 * @file Protocol.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-08-14
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SNOWY_PROTOCOL_H_
#define SNOWY_PROTOCOL_H_

#include <memory>
#include <sstream>
#include <string>

#include "ByteArray.hpp"
/*
 * 私有通信协议
 * 第一个字节是魔法数。
 * 第二个字节代表协议版本号，以便对协议进行扩展，使用不同的协议解析器。
 * 第三个字节是请求类型，如心跳包，rpc请求。
 * 第四个字节开始是一个32位序列号。
 * 第七个字节开始的四字节表示消息长度，即后面要接收的内容长度。
 */

class Protocol {
public:
  static constexpr uint8_t MAGIC = 0xcc;
  static constexpr uint8_t DEFAULT_VERSION = 0x01;
  static constexpr uint8_t BASE_LENGTH = 11;
  enum class MsgType : uint8_t {
    HEARTBEAT_PACKET, // 心跳包
    RPC_PROVIDER,     // 向服务中心声明为provider
    RPC_CONSUMER,     // 向服务中心声明为consumer

    RPC_REQUEST,  // 通用请求
    RPC_RESPONSE, // 通用响应

    RPC_METHOD_REQUEST,  // 请求方法调用
    RPC_METHOD_RESPONSE, // 响应方法调用

    RPC_SERVICE_REGISTER, // 向中心注册服务
    RPC_SERVICE_REGISTER_RESPONSE,

    RPC_SERVICE_DISCOVER, // 向中心请求服务发现
    RPC_SERVICE_DISCOVER_RESPONSE,

    RPC_SUBSCRIBE_REQUEST, // 订阅
    RPC_SUBSCRIBE_RESPONSE,

    RPC_PUBLISH_REQUEST, // 发布
    RPC_PUBLISH_RESPONSE
  };

private:
  uint8_t magic_ = MAGIC;
  uint8_t version_ = DEFAULT_VERSION;
  uint8_t type_ = 0;
  uint32_t sequence_id_ = 0;
  uint32_t content_length_ = 0;
  std::string content_;

public:
  static std::shared_ptr<Protocol>
  Create(MsgType type, const std::string &content, uint32_t id = 0) {
    std::shared_ptr<Protocol> proto = std::make_shared<Protocol>();
    proto->setMsgType(type);
    proto->setContent(content);
    proto->setSequenceId(id);
    return proto;
  }

  static std::shared_ptr<Protocol> HeartBeat() {
    static std::shared_ptr<Protocol> heartbeat =
        Protocol::Create(Protocol::MsgType::HEARTBEAT_PACKET, "");
    return heartbeat;
  }

  std::string toString() {
    std::stringstream ss;
    ss << "[ magic=" << magic_ << " version=" << version_ << " type=" << type_
       << " id=" << sequence_id_ << " length=" << content_length_
       << " content=" << content_ << " ]";
    return ss.str();
  }

public:
  uint8_t getMagic() const { return magic_; }
  uint8_t getVersion() const { return version_; }
  MsgType getMsgType() const { return static_cast<MsgType>(type_); }
  uint32_t getSequenceId() const { return sequence_id_; }
  uint32_t getContentLength() const { return content_length_; }
  const std::string &getContent() const { return content_; }
  void setMagic(uint8_t magic) { magic_ = magic; }
  void setVersion(uint8_t version) { version_ = version; }
  void setMsgType(MsgType type) { type_ = static_cast<uint8_t>(type); }
  void setSequenceId(uint32_t id) { sequence_id_ = id; }
  void setContentLength(uint32_t len) { content_length_ = len; }
  void setContent(const std::string &content) { content_ = content; }

  // TODO 编码 解码
public:
  std::shared_ptr<ByteArray> encodeMeta() {
    auto bt = std::make_shared<ByteArray>();
    bt->writeFuint8(magic_);
    bt->writeFuint8(version_);
    bt->writeFuint8(type_);
    bt->writeFuint32(sequence_id_);
    bt->writeFuint32(content_.size());
    return bt;
  }

  std::shared_ptr<ByteArray> encode() {
    auto bt = std::make_shared<ByteArray>();
    bt->writeFuint8(magic_);
    bt->writeFuint8(version_);
    bt->writeFuint8(type_);
    bt->writeFuint32(sequence_id_);
    bt->writeStringF32(content_);
    return bt;
  }

  void decodeMeta(std::shared_ptr<ByteArray> bt) {
    magic_ = bt->readFuint8();
    version_ = bt->readFuint8();
    type_ = bt->readFuint8();
    sequence_id_ = bt->readFuint32();
    content_length_ = bt->readFuint32();
  }

  void decode(std::shared_ptr<ByteArray> bt) {
    magic_ = bt->readFuint8();
    version_ = bt->readFuint8();
    type_ = bt->readFuint8();
    sequence_id_ = bt->readFuint32();
    content_ = bt->readStringF32();
    content_length_ = content_.size();
  }
};
#endif