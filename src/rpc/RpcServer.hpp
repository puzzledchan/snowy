/**
 * @file RpcServer.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-08-14
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SNOWY_RPCSERVER_H
#define SNOWY_RPCSERVER_H

#include <string>

#include <functional>
#include <memory>

#include <map>

#include "Rpc.hpp"
#include "RpcSession.hpp"
#include "Serializer.hpp"
#include "TcpServer.hpp"
#include "Traits.hpp"

class RpcServer : public TcpServer {
private:
  // 序列化 与 参数
  std::map<std::string, std::function<void(Serializer, const std::string &)>>
      handlers_;

  std::shared_ptr<RpcSession> register_; // 注册中心
  uint32_t alive_time_;                  // 客户端的心跳时间
  std::unordered_multimap<std::string, std::weak_ptr<RpcSession>> subscribes_;

public:
  /**
   * @brief 处理客户端过程调用请求
   */
  std::shared_ptr<Protocol> handleMethodCall(std::shared_ptr<Protocol> proto);
  /**
   * @brief 处理心跳包
   */
  std::shared_ptr<Protocol>
  handleHeartbeatPacket(std::shared_ptr<Protocol> proto);

  template <typename Func>
  void registerMethod(const std::string &method, Func func) {
    handlers_[method] = [func, this](Serializer serializer,
                                     const std::string &arg) {
      proxy(func, serializer, arg);
    };
  }

public:
  void makeNewConnection(int connfd, const sockaddr_in &peer) override;

protected:
  /**
   * @brief 调用服务端注册的函数，返回序列化完的结果
   * @param[in] name 函数名
   * @param[in] arg 函数参数字节流
   * @return 函数调用的序列化结果
   */
  Serializer call(const std::string &name, const std::string &arg);

  /**
   * @brief 调用代理
   * @param[in] fun 函数
   * @param[in] serializer 返回调用结果
   * @param[in] arg 函数参数字节流
   */
  template <typename F>
  void proxy(F fun, Serializer serializer, const std::string &arg) {
    typename function_traits<F>::stl_function_type func(fun);
    using Return = typename function_traits<F>::return_type;
    using Args = typename function_traits<F>::tuple_type;

    Serializer s(arg);
    // 反序列化字节流，存为参数tuple
    Args args;
    s >> args;

    return_type_t<Return> rt{};

    constexpr auto size =
        std::tuple_size<typename std::decay<Args>::type>::value;
    auto invoke =
        [&func, &args ]<std::size_t... Index>(std::index_sequence<Index...>) {
      return func(std::get<Index>(std::forward<Args>(args))...);
    };

    if constexpr (std::is_same_v<Return, void>) {
      invoke(std::make_index_sequence<size>{});
    } else {
      rt = invoke(std::make_index_sequence<size>{});
    }

    Result<Return> val;
    val.setCode(RPC_SUCCESS);
    val.setVal(rt);
    serializer << val;
  }
};

#endif