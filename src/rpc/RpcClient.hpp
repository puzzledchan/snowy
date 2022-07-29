/**
 * @file RpcClient.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-08-20
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SNOWY_RPCCLIENT_H
#define SNOWY_RPCCLIENT_H

#include "Protocol.hpp"
#include "Rpc.hpp"
#include "RpcSession.hpp"
#include "Socket.hpp"
#include <condition_variable>
#include <future>
#include <mutex>

class RpcClient {
private:
  std::mutex cli_mutex_;
  // 序列号到对应调用者协程的 Channel 映射
  std::map<uint32_t, std::shared_ptr<std::promise<std::shared_ptr<Protocol>>>>
      sessionHandle_;
  std::shared_ptr<RpcSession> rpc_session_;
  std::vector<std::shared_ptr<EventLoop>> loops_;
  std::vector<std::thread> thread_pool_;
  std::atomic<uint32_t> sequenceId_ = 0;
  // std::shared_ptr<EventLoop> loop_;

public:
  RpcClient() {}
  ~RpcClient() {}
  void start();
  bool connect();
  /**
   * @brief 有参数的调用
   * @param[in] name 函数名
   * @param[in] ps 可变参
   * @return 返回调用结果
   */
  template <typename R, typename... Params>
  Result<R> call(const std::string &name, Params... ps) {
    using args_type = std::tuple<typename std::decay<Params>::type...>;
    args_type args = std::make_tuple(ps...);
    Serializer s;
    s << name << args;
    return call<R>(s);
  }
  /**
   * @brief 无参数的调用
   * @param[in] name 函数名
   * @return 返回调用结果
   */
  template <typename R> Result<R> call(const std::string &name) {
    Serializer s;
    s << name;
    return call<R>(s);
  }

public:
  /**
   * @brief 通过序列号获取对应调用者的 Channel，将 response 放入 Channel
   * 唤醒调用者
   */
  void handleMethodResponse(std::shared_ptr<Protocol> response);

private:
  template <typename R> Result<R> call(Serializer s) {
    Result<R> val;
    auto request = Protocol::Create(Protocol::MsgType::RPC_METHOD_REQUEST,
                                    s.toString(), sequenceId_);
    auto promise = std::make_shared<std::promise<std::shared_ptr<Protocol>>>();
    sessionHandle_[sequenceId_] = promise;
    ++sequenceId_;
    rpc_session_->sendProtocol(request);
    auto f = promise->get_future();
    auto response = f.get();
    Serializer serializer(response->getContent());
    serializer >> val;
    return val;
  }

private:
  void _startWorkers();
  std::shared_ptr<EventLoop> _getNextLoop();
};
#endif