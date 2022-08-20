#include "Socket.hpp"

int CreateTCPSocket() { return ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); }

void SetNonBlock(int sock, bool nonblock) {
  int flag = ::fcntl(sock, F_GETFL, 0);
  assert(flag >= 0 && "Non Block failed");

  if (nonblock)
    flag = ::fcntl(sock, F_SETFL, flag | O_NONBLOCK);
  else
    flag = ::fcntl(sock, F_SETFL, flag & ~O_NONBLOCK);
}
