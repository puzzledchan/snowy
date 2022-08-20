/**
 * @file SystemCall.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-08-02
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SNOWY_SYSTEMCALL_HPP
#define SNOWY_SYSTEMCALL_HPP

#include <sys/syscall.h>
#include <unistd.h>

#include <thread>

pid_t GetThreadId() {
  //返回内核中tid  不使用pthread_self()是因为其返回的不是真正线程ID
  return syscall(SYS_gettid);
}
std::thread::id CppGetThreadId() {
  //返回内核中tid  不使用pthread_self()是因为其返回的不是真正线程ID
  return std::this_thread::get_id();
}
#endif