/**
 * @file RpcSession.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-08-14
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SNOWY_RPCSESSION_H
#define SNOWY_RPCSESSION_H

#include <memory>
#include <mutex>

#include "Connection.hpp"
#include "Protocol.hpp"

class RpcServer;

class RpcSession : public Connection {
private:
  std::mutex pro_mutex_;
  std::shared_ptr<RpcServer> server_;
  using handleRequestResponse =
      std::function<std::shared_ptr<Protocol>(std::shared_ptr<Protocol>)>;
  handleRequestResponse handleMethodCall;

  using handleResponse = std::function<void(std::shared_ptr<Protocol>)>;
  handleResponse handleMethodResponce;

public:
  // SOCKET
  RpcSession(std::shared_ptr<EventLoop> loop) : Connection(loop) {}

public:
  std::shared_ptr<Protocol> recvProtocol();
  void sendProtocol(std::shared_ptr<Protocol> proto);
  void processMessage() override;

public:
  /**
   * @brief 处理客户端过程调用请求
   */
  void sethandleMethodCall(handleRequestResponse func) {
    handleMethodCall = func;
  }
  void sethandleMethodResponse(handleResponse func) {
    handleMethodResponce = func;
  }
};
#endif