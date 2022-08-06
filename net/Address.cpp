

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <sys/types.h>

#include "Address.hpp"

int Address::getFamily() const { return getAddr()->sa_family; }

Address::ptr Address::CreateFromText(const struct sockaddr *sockaddr,
                                     socklen_t socklen) {
  if (sockaddr == nullptr)
    return nullptr;

  Address::ptr result;
  switch (sockaddr->sa_family) {
  // IPv4类型地址
  case AF_INET:
    result.reset(new IPv4Address(*(const struct sockaddr_in *)sockaddr));
    break;

  // IPv6类型地址
  case AF_INET6:
    result.reset(new IPv6Address(*(const struct sockaddr_in6 *)sockaddr));
    break;

  // Unix域类型地址
  case AF_UNIX:
    result.reset(new UnixAddress(*(const struct sockaddr_un *)sockaddr));
    break;

  //其他为未知类型地址
  default:
    result.reset(new UnkonwAddress(*sockaddr));
    break;
  }
  return result;
}

/*******************************************IPAddress****************************************/

IPAddress::ptr IPAddress::CreateFromText(const char *address, uint16_t port) {
  struct addinfo hints, *results;
}
