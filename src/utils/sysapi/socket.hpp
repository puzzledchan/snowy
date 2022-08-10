#include <cassert>
#include <fcntl.h>

void SetNonBlock(int sock, bool nonblock = true) {
  int flag = ::fcntl(sock, F_GETFL, 0);
  assert(flag >= 0 && "Non Block failed");

  if (nonblock)
    flag = ::fcntl(sock, F_SETFL, flag | O_NONBLOCK);
  else
    flag = ::fcntl(sock, F_SETFL, flag & ~O_NONBLOCK);
}