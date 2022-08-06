/**
 * @file Timer.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-08-03
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SNOWY_TIMER_H
#define SNOWY_TIMER_H
#include <chrono>
#include <memory>

class Timer {

public:
  bool cancel();
  bool refresh();
  bool reset();
};

class TimerManager final {
public:
  TimerManager() {}
  ~TimerManager() {}

  TimerManager(const TimerManager &) = delete;
  void operator=(const TimerManager &) = delete;
};

#endif