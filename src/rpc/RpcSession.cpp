/**
 * @file RpcSession.cpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-08-14
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "RpcSession.hpp"

std::shared_ptr<Protocol> RpcSession::recvProtocol() {
  auto proto = std::make_shared<Protocol>();
  auto byteArray = std::make_shared<ByteArray>();
  if (recv_buf_.readableSize() < proto->BASE_LENGTH) {
    return nullptr;
  }
  auto base_length =
      recv_buf_.peekDataAt(byteArray->writeAddr(), proto->BASE_LENGTH);
  byteArray->produce(base_length);
  proto->decodeMeta(byteArray);
  if (proto->getMagic() != Protocol::MAGIC) {
    return nullptr;
  }

  auto content_length = proto->getContentLength();
  // 心跳包
  if (!content_length) {
    recv_buf_.consume(base_length);
    return proto;
  }
  if (recv_buf_.readableSize() < proto->BASE_LENGTH + content_length) {
    return nullptr;
  }
  recv_buf_.consume(base_length);
  std::string buff;
  buff.resize(proto->getContentLength());
  auto len = recv_buf_.popData(&buff[0], proto->getContentLength());
  assert(len == proto->getContentLength());
  proto->setContent(buff);
  return proto;
}

// SafeSendProtocol
void RpcSession::sendProtocol(std::shared_ptr<Protocol> proto) {
  // encode
  auto func = [proto, this]() {
    std::shared_ptr<ByteArray> ByteArray = proto->encode();
    std::lock_guard<std::mutex> lock(pro_mutex_);
    send_buf_.pushData(ByteArray->readAddr(), ByteArray->readableSize());
    loop_->Modify(EPOLL_ET_Write | EPOLL_ET_Read, shared_from_this());
  };
  loop_->RunInThisLoop(func);
}

void RpcSession::processMessage() {
  // 接受消息
  auto proto = recvProtocol();
  if (proto == nullptr)
    return;
  std::shared_ptr<Protocol> response;
  Protocol::MsgType type = proto->getMsgType();
  switch (type) {
  case Protocol::MsgType::HEARTBEAT_PACKET:
    break;
  case Protocol::MsgType::RPC_SUBSCRIBE_REQUEST:
    break;
  case Protocol::MsgType::RPC_METHOD_REQUEST:
    if (handleMethodCall) {
      response = handleMethodCall(proto);
      sendProtocol(response);
    }
    break;
  case Protocol::MsgType::RPC_METHOD_RESPONSE:
    if (handleMethodResponce) {
      handleMethodResponce(proto);
    }
    break;
  default:
    break;
  }
}
