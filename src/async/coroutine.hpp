/**
 * @file coroutine.hpp
 * @author JDongChen
 * @brief 协程任务
 * @version 0.1
 * @date 2022-08-14
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SNOWY_COROUTINUE_H
#define SNOWY_COROUTINUE_H

#include <coroutine>
#include <exception>

template <typename T> struct Task {
  struct promise_type;
  using handle = std::coroutine_handle<promise_type>;
  struct promise_type {
    Task get_return_object() {
      return {std::coroutine_handle<promise_type>::from_promise(*this)};
    }
    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_value(T val) { val_ = val; }
    std::suspend_always yield_value(T val) {
      val_ = val;
      return {};
    }
    void unhandled_exception() { std::terminate(); }
    T val_;
  };
  Task() : handle_(nullptr) {}
  Task(handle h) : handle_(h) {}
  Task(const Task &other) = delete;
  Task(Task &&other) noexcept : handle_(other.handle_) {
    other.handle_ = nullptr;
  }
  Task &operator=(const Task &rhs) = delete;
  Task &operator=(Task &&rhs) noexcept {
    if (std::addressof(rhs) != this) {
      if (handle_) {
        handle_.destroy();
      }

      handle_ = rhs.handle_;
      rhs.handle_ = nullptr;
    }

    return *this;
  }
  T get() { return handle_.promise().val_; }
  void resume() { handle_.resume(); }
  bool done() { return !handle_ || handle_.done(); }
  bool destroy() {
    handle_.destroy();
    handle_ = nullptr;
  }
  ~Task() {
    if (handle_) {
      handle_.destroy();
    }
  }

private:
  handle handle_ = nullptr;
};

template <> struct Task<void> {
  struct promise_type;
  using handle = std::coroutine_handle<promise_type>;
  struct promise_type {
    Task get_return_object() {
      return {std::coroutine_handle<promise_type>::from_promise(*this)};
    }
    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() {}
    std::suspend_always yield_value() { return {}; }
    void unhandled_exception() { std::terminate(); }
  };

  Task() : handle_(nullptr) {}
  Task(handle h) : handle_(h) {}
  Task(const Task &rhs) = delete;
  Task(Task &&rhs) noexcept : handle_(rhs.handle_) { rhs.handle_ = nullptr; }
  Task &operator=(const Task &rhs) = delete;
  Task &operator=(Task &&rhs) noexcept {
    if (std::addressof(rhs) != this) {
      if (handle_) {
        handle_.destroy();
      }

      handle_ = rhs.handle_;
      rhs.handle_ = nullptr;
    }

    return *this;
  }
  void resume() { handle_.resume(); }
  bool done() { return !handle_ || handle_.done(); }
  void destroy() {
    handle_.destroy();
    handle_ = nullptr;
  }
  ~Task() {
    if (handle_) {
      handle_.destroy();
    }
  }
  handle handle_ = nullptr;
};

#endif