/**
 * @file singleton.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-07-30
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SNOWY_SINGLETON_H
#define SNOWY_SINGLETON_H

#include <memory>

/**
 * @brief 单例模式 返回对象
 *
 * @tparam T
 */
template <typename T> class Singleton {
public:
  static T *GetInstance() {
    static T instance;
    return &instance;
  }
  Singleton(T &&) = delete;
  Singleton(const T &) = delete;
  void operator=(const T &) = delete;
};
/**
 * @brief 单例模式 返回对象的共享智能指针
 *
 * @tparam T
 */
template <typename T> class SingletonPtr {
public:
  static std::shared_ptr<T> GetInstance() {
    static std::shared_ptr<T> instance(std::make_shared<T>());
    return instance;
  }
  SingletonPtr(T &&) = delete;
  SingletonPtr(const T &) = delete;
  void operator=(const T &) = delete;
};

#endif