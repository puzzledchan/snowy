#include "SystemCall.hpp"

pid_t GetThreadId() {
  //返回内核中tid  不使用pthread_self()是因为其返回的不是真正线程ID
  return syscall(SYS_gettid);
}
std::thread::id CppGetThreadId() {
  //返回内核中tid  不使用pthread_self()是因为其返回的不是真正线程ID
  return std::this_thread::get_id();
}