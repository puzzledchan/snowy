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

pid_t GetThreadId();
std::thread::id CppGetThreadId();

#endif