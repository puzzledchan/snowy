/**
 * @file Rpc.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-08-19
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SNOWY_RPC_H
#define SNOWY_RPC_H

#include <memory>

#include "Serializer.hpp"

// 连接池向注册中心订阅的前缀
inline const char *RPC_SERVICE_SUBSCRIBE = "[[rpc service subscribe]]";

template <typename T> struct return_type { using type = T; };

template <> struct return_type<void> { using type = int8_t; };

/**
 * @brief 调用结果为 void 类型的，将类型转换为 int8_t
 */
template <typename T> using return_type_t = typename return_type<T>::type;

/**
 * @brief RPC调用状态
 */
enum RpcState {
  RPC_SUCCESS = 0, // 成功
  RPC_FAIL,        // 失败
  RPC_NO_METHOD,   // 没有找到调用函数
  RPC_CLOSED,      // RPC 连接被关闭
  RPC_TIMEOUT      // RPC 调用超时
};

template <typename T = void> class Result {
public:
  using ret_type = return_type_t<T>;
  using msg_type = std::string;
  using code_type = uint16_t;
  Result() {}
  bool valid() const { return code_ == 0; }
  ret_type &getVal() { return val_; }
  void setVal(const ret_type &val) { val_ = val; }
  void setCode(code_type code) { code_ = code; }
  int getCode() const { return code_; }
  void setMsg(msg_type msg) { msg_ = msg; }
  const msg_type &getMsg() const { return msg_; }

public:
  ret_type *operator->() noexcept { return &val_; }
  const ret_type *operator->() const noexcept { return &val_; }

public:
  static Result<T> Success() {
    Result<T> res;
    res.setCode(RPC_SUCCESS);
    res.setMsg("success");
    return res;
  }
  static Result<T> Fail() {
    Result<T> res;
    res.setCode(RPC_FAIL);
    res.setMsg("fail");
    return res;
  }

  /**
   * @brief 反序列化回 Result
   * @param[in] in 序列化的结果
   * @param[in] d 反序列化回 Result
   * @return Serializer
   */
  friend Serializer &operator>>(Serializer &in, Result<T> &d) {
    in >> d.code_ >> d.msg_;
    if (d.code_ == 0) {
      in >> d.val_;
    }
    return in;
  }

  /**
   * @brief 将 Result 序列化
   * @param[in] out 序列化输出
   * @param[in] d 将 Result 序列化
   * @return Serializer
   */
  friend Serializer &operator<<(Serializer &out, Result<T> d) {
    out << d.code_ << d.msg_ << d.val_;
    return out;
  }

private:
  code_type code_ = 0;
  msg_type msg_;
  ret_type val_;
};
#endif