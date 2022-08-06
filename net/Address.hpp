/**
 * @file Address.hpp
 * @author JDongChen
 * @brief 通信地址类
 * @version 0.1
 * @date 2022-08-04
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SNOWY_ADDRESS_H
#define SNOWY_ADDRESS_H

#include <netinet/in.h>
#include <sys/socket.h>

#include <memory>

class Address {
public:
  using ptr = std::shared_ptr<Address>;
  virtual ~Address(){};
  int getFamily() const;

  virtual const struct sockaddr *getAddr() const = 0;
  virtual struct sockaddr *getAddr() = 0;
  virtual socklen_t getAddrLen() const = 0;

public:
  ///@brief 输出通信地址信息
  virtual std::ostream &insert(std::ostream &os) const = 0;
  ///@brief 从文本创建对象
  static Address::ptr CreateFromText(const struct sockaddr *sockaddr,
                                     socklen_t len);
};

class IPAddress : public Address {
public:
  using ptr = std::shared_ptr<IPAddress>;
  virtual IPAddress::ptr broadcastAddress(uint32_t len) = 0;

  virtual IPAddress::ptr subnetAddress(uint32_t len) = 0;
  virtual IPAddress::ptr subnetMask(uint32_t len) = 0;

  ///@brief 返回端口号与设置端口号
  virtual uint16_t getPort() const = 0;
  virtual void setPort(uint16_t val) = 0;

  ///@brief 从字符串转化为实际IP地址
  static IPAddress::ptr CreateFromText(const char *address, uint16_t port = 0);
};

class IPv4Address : public IPAddress {
private:
  struct sockaddr_in sockaddr_;

public:
  typedef std::shared_ptr<IPv4Address> ptr;

  IPv4Address(const struct sockaddr_in sockaddr);
  IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);

  const struct sockaddr *getAddr() const override;
  sockaddr *getAddr() override;
  socklen_t getAddrLen() const override;

  std::ostream &insert(std::ostream &os) const override;

  IPAddress::ptr broadcastAddress(uint32_t len) override;
  IPAddress::ptr subnetAddress(uint32_t len) override;
  IPAddress::ptr subnetMask(uint32_t len) override;

  ///@brief 端口设置
  uint16_t getPort() const override;
  void setPort(uint16_t val) override;

  ///@brief 文本转化
  static IPv4Address::ptr CreateFromText(const char *addr, uint16_t port = 0);
};

class IPv6Address : public IPAddress {};
class UnixAddress : public IPAddress {};
class UnkonwAddress : public IPAddress {};
#endif