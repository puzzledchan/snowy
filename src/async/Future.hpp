#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>

#include "Scheduler.hpp"
#include "Try.hpp"

enum class Progress {
  None,
  Timeout,
  Done,
  Retrieved,
};

using TimeoutCallback = std::function<void()>;

/**
 * @brief future 和 promise 中的共享状态
 *
 * @tparam T
 */
template <typename T> struct State {
  static_assert(std::is_same<T, void>::value ||
                    std::is_copy_constructible<T>() ||
                    std::is_move_constructible<T>(),
                "must be copyable or movable or void");

  State() : progress_(Progress::None), retrieved_{false} {}

  std::mutex thenLock_;

  using ValueType = typename TryWrapper<T>::Type;
  ValueType value_;
  // 闭包函数
  std::function<void(ValueType &&)> then_;
  Progress progress_;

  std::function<void(TimeoutCallback &&)> onTimeout_;
  std::atomic<bool> retrieved_;
};

template <typename T> class Future;
/**
 * @brief promise 承诺提供一个值
 * SetException 设置异常
 * SetValue 提供值，调用state_->next()
 * @tparam T 提供参数类型
 */
template <typename T> class Promise {
public:
  Promise() : state_(std::make_shared<State<T>>()) {}

  // The lambda with movable capture can not be stored in
  // std::function, just for compile, do NOT copy promise!
  Promise(const Promise &) = default;
  Promise &operator=(const Promise &) = default;

  Promise(Promise &&pm) = default;
  Promise &operator=(Promise &&pm) = default;

  void SetException(std::exception_ptr exp) {
    std::unique_lock<std::mutex> guard(state_->thenLock_);
    if (state_->progress_ != Progress::None)
      return;

    state_->progress_ = Progress::Done;
    state_->value_ = typename State<T>::ValueType(std::move(exp));
    guard.unlock();

    if (state_->then_)
      state_->then_(
          std::move(state_->value_)); // 闭包函数调用；参数为TryWrapper<T>::type
  }

  template <typename SHIT = T>
  typename std::enable_if<!std::is_void<SHIT>::value, void>::type
  SetValue(SHIT &&t) {
    // If ThenImp is running, here will wait for the lock.
    // After set then_, ThenImp will release lock.
    // And this func got lock, definitely will call then_.
    std::unique_lock<std::mutex> guard(state_->thenLock_);
    if (state_->progress_ != Progress::None)
      return;

    // ! 完美转发，参数不为空
    state_->progress_ = Progress::Done;
    state_->value_ = std::forward<SHIT>(t);

    guard.unlock();

    // When reach here, state_ is determined, so mutex is useless
    // If the ThenImp function run, it'll see the Done state and
    // call user func there, not assign to then_.
    if (state_->then_)
      state_->then_(std::move(state_->value_));
  }

  template <typename SHIT = T>
  typename std::enable_if<std::is_void<SHIT>::value, void>::type SetValue() {
    std::unique_lock<std::mutex> guard(state_->thenLock_);
    if (state_->progress_ != Progress::None)
      return;
    // ! 参数为空
    state_->progress_ = Progress::Done;
    state_->value_ = Try<void>();

    guard.unlock();
    if (state_->then_)
      state_->then_(std::move(state_->value_));
  }

  Future<T> GetFuture() {
    bool expect = false;
    if (!state_->retrieved_.compare_exchange_strong(expect, true)) {
      throw std::runtime_error("Future already retrieved");
    }

    return Future<T>(state_);
  }

  bool IsReady() const { return state_->progress_ != Progress::None; }

private:
  std::shared_ptr<State<T>> state_;
};

/********************* Future *********************************/
// Make exception future
template <typename T, typename E>
inline Future<T> MakeExceptionFuture(E &&exp) {
  Promise<T> pm;
  pm.SetException(std::make_exception_ptr(std::forward<E>(exp)));
  return pm.GetFuture();
}

template <typename T>
inline Future<T> MakeExceptionFuture(std::exception_ptr &&eptr) {
  Promise<T> pm;
  pm.SetException(std::move(eptr));
  return pm.GetFuture();
}

template <typename T> class Future {
public:
  using InnerType = T;

  template <typename U> friend class Future;

  Future() = default;
  Future(Future &&fut) = default;
  Future &operator=(Future &&fut) = default;

  Future(const Future &) = delete;
  void operator=(const Future &) = delete;

  explicit Future(std::shared_ptr<State<T>> state) : state_(std::move(state)) {}
  bool valid() const { return state_ != nullptr; }

  // The blocking interface
  // PAY ATTENTION to deadlock: Wait thread must NOT be same as promise
  // thread!!!

  // future 在共享状态的锁上休眠，直到唤醒
  typename State<T>::ValueType Wait(const std::chrono::milliseconds &timeout =
                                        std::chrono::milliseconds(24 * 3600 *
                                                                  1000)) {

    std::unique_lock<std::mutex> guard(state_->thenLock_);
    switch (state_->progress_) {
    case Progress::None:
      break;

    case Progress::Timeout:
      throw std::runtime_error("Future timeout");

    case Progress::Done:
      state_->progress_ = Progress::Retrieved;
      return std::move(state_->value_);

    default:
      throw std::runtime_error("Future already retrieved");
    }
    guard.unlock();

    auto cond(std::make_shared<std::condition_variable>());
    auto mutex(std::make_shared<std::mutex>());
    bool ready = false;
    typename State<T>::ValueType value;

    this->Then([&value, &ready,
                wcond = std::weak_ptr<std::condition_variable>(cond),
                wmutex = std::weak_ptr<std::mutex>(mutex)](
                   typename State<T>::ValueType &&v) {
      auto cond = wcond.lock();
      auto mutex = wmutex.lock();
      if (!cond || !mutex)
        return;

      std::unique_lock<std::mutex> guard(*mutex);
      value = std::move(v);
      ready = true;
      cond->notify_one();
    });

    std::unique_lock<std::mutex> waiter(*mutex);
    bool success =
        cond->wait_for(waiter, timeout, [&ready]() { return ready; });
    if (success)
      return std::move(value);
    else
      throw std::runtime_error("Future wait_for timeout");
  }

  // T is of type Future<InnerType>
  template <typename SHIT = T>
  typename std::enable_if<IsFuture<SHIT>::value, SHIT>::type Unwrap() {
    using InnerType = typename IsFuture<SHIT>::Inner;

    static_assert(std::is_same<SHIT, Future<InnerType>>::value, "Kidding me?");

    Promise<InnerType> prom;
    Future<InnerType> fut = prom.GetFuture();

    std::unique_lock<std::mutex> guard(state_->thenLock_);
    if (state_->progress_ == Progress::Timeout) {
      throw std::runtime_error("Wrong state : Timeout");
    } else if (state_->progress_ == Progress::Done) {
      try {
        auto innerFuture = std::move(state_->value_);
        return std::move(innerFuture.Value());
      } catch (const std::exception &e) {
        return MakeExceptionFuture<InnerType>(std::current_exception());
      }
    } else {
      _SetCallback([pm = std::move(prom)](
                       typename TryWrapper<SHIT>::Type &&innerFuture) mutable {
        try {
          SHIT future = std::move(innerFuture);
          future._SetCallback(
              [pm = std::move(pm)](
                  typename TryWrapper<InnerType>::Type &&t) mutable {
                // No need scheduler here, think about this code:
                // `outer.Unwrap().Then(sched, func);`
                // outer.Unwrap() is the inner future, the below line
                // will trigger func in sched thread.
                pm.SetValue(std::move(t));
              });
        } catch (...) {
          pm.SetException(std::current_exception());
        }
      });
    }

    return fut;
  }

  /**
   * @brief
   *
   * @tparam F
   * @tparam R
   * @tparam T>
   * @param f
   * @return R::ReturnFutureType
   */
  template <typename F, typename R = CallableResult<F, T>>
  auto Then(F &&f) -> typename R::ReturnFutureType {
    typedef typename R::Arg Arguments;
    return _ThenImpl<F, R>(nullptr, std::forward<F>(f), Arguments());
  }

  // f will be called in sched
  template <typename F, typename R = CallableResult<F, T>>
  auto Then(Scheduler *sched, F &&f) -> typename R::ReturnFutureType {
    typedef typename R::Arg Arguments;
    return _ThenImpl<F, R>(sched, std::forward<F>(f), Arguments());
  }

  // 1. F does not return future type
  template <typename F, typename R, typename... Args>
  typename std::enable_if<!R::IsReturnsFuture::value,
                          typename R::ReturnFutureType>::type
  _ThenImpl(Scheduler *sched, F &&f, ResultOfWrapper<F, Args...>) {
    static_assert(sizeof...(Args) <= 1, "Then must take zero/one argument");

    using FReturnType = typename R::IsReturnsFuture::Inner;

    Promise<FReturnType> pm;
    auto nextFuture = pm.GetFuture();

    using FuncType = typename std::decay<F>::type;

    std::unique_lock<std::mutex> guard(state_->thenLock_);
    if (state_->progress_ == Progress::Timeout) {
      throw std::runtime_error("Wrong state : Timeout");
    } else if (state_->progress_ == Progress::Done) {
      typename TryWrapper<T>::Type t;
      try {
        t = std::move(state_->value_);
      } catch (const std::exception &e) {
        t = (typename TryWrapper<T>::Type)(std::current_exception());
      }

      guard.unlock();

      if (sched) {
        sched->Schedule([t = std::move(t), f = std::forward<FuncType>(f),
                         pm = std::move(pm)]() mutable {
          auto result = WrapWithTry(f, std::move(t));
          pm.SetValue(std::move(result));
        });
      } else {
        auto result = WrapWithTry(f, std::move(t));
        pm.SetValue(std::move(result));
      }
    } else {
      // set this future's then callback
      _SetCallback(
          [sched, func = std::forward<FuncType>(f),
           prom = std::move(pm)](typename TryWrapper<T>::Type &&t) mutable {
            if (sched) {
              sched->Schedule([func = std::move(func), t = std::move(t),
                               prom = std::move(prom)]() mutable {
                // run callback, T can be void, thanks to folly
                auto result = WrapWithTry(func, std::move(t));
                // set next future's result
                prom.SetValue(std::move(result));
              });
            } else {
              // run callback, T can be void, thanks to folly Try<>
              auto result = WrapWithTry(func, std::move(t));
              // set next future's result
              prom.SetValue(std::move(result));
            }
          });
    }

    return std::move(nextFuture);
  }

  // 2. F return another future type
  template <typename F, typename R, typename... Args>
  typename std::enable_if<R::IsReturnsFuture::value,
                          typename R::ReturnFutureType>::type
  _ThenImpl(Scheduler *sched, F &&f, ResultOfWrapper<F, Args...>) {
    static_assert(sizeof...(Args) <= 1, "Then must take zero/one argument");

    using FReturnType = typename R::IsReturnsFuture::Inner;

    Promise<FReturnType> pm;
    auto nextFuture = pm.GetFuture();

    using FuncType = typename std::decay<F>::type;

    std::unique_lock<std::mutex> guard(state_->thenLock_);
    if (state_->progress_ == Progress::Timeout) {
      throw std::runtime_error("Wrong state : Timeout");
    } else if (state_->progress_ == Progress::Done) {
      typename TryWrapper<T>::Type t;
      try {
        t = std::move(state_->value_);
      } catch (const std::exception &e) {
        t = decltype(t)(std::current_exception());
      }

      guard.unlock();

      auto cb = [res = std::move(t), f = std::forward<FuncType>(f),
                 prom = std::move(pm)]() mutable {
        // because func return another future: innerFuture, when innerFuture is
        // done, nextFuture can be done
        decltype(f(res.template Get<Args>()...)) innerFuture;
        if (res.HasException()) {
          // Failed if Args... is void
          innerFuture =
              f(typename TryWrapper<typename std::decay<Args...>::type>::Type(
                  res.Exception()));
        } else {
          innerFuture = f(res.template Get<Args>()...);
        }

        if (!innerFuture.valid()) {
          return;
        }

        std::unique_lock<std::mutex> guard(innerFuture.state_->thenLock_);
        if (innerFuture.state_->progress_ == Progress::Timeout) {
          throw std::runtime_error("Wrong state : Timeout");
        } else if (innerFuture.state_->progress_ == Progress::Done) {
          typename TryWrapper<FReturnType>::Type t;
          try {
            t = std::move(innerFuture.state_->value_);
          } catch (const std::exception &e) {
            t = decltype(t)(std::current_exception());
          }

          guard.unlock();
          prom.SetValue(std::move(t));
        } else {
          innerFuture._SetCallback(
              [prom = std::move(prom)](
                  typename TryWrapper<FReturnType>::Type &&t) mutable {
                prom.SetValue(std::move(t));
              });
        }
      };

      if (sched)
        sched->Schedule(std::move(cb));
      else
        cb();
    } else {
      // set this future's then callback
      _SetCallback([sched = sched, func = std::forward<FuncType>(f),
                    prom = std::move(pm)](
                       typename TryWrapper<T>::Type &&t) mutable {
        auto cb = [func = std::move(func), t = std::move(t),
                   prom = std::move(prom)]() mutable {
          // because func return another future: innerFuture, when innerFuture
          // is done, nextFuture can be done
          decltype(func(t.template Get<Args>()...)) innerFuture;
          if (t.HasException()) {
            // Failed if Args... is void
            innerFuture = func(
                typename TryWrapper<typename std::decay<Args...>::type>::Type(
                    t.Exception()));
          } else {
            innerFuture = func(t.template Get<Args>()...);
          }

          if (!innerFuture.valid()) {
            return;
          }
          std::unique_lock<std::mutex> guard(innerFuture.state_->thenLock_);
          if (innerFuture.state_->progress_ == Progress::Timeout) {
            throw std::runtime_error("Wrong state : Timeout");
          } else if (innerFuture.state_->progress_ == Progress::Done) {
            typename TryWrapper<FReturnType>::Type t;
            try {
              t = std::move(innerFuture.state_->value_);
            } catch (const std::exception &e) {
              t = decltype(t)(std::current_exception());
            }

            guard.unlock();
            prom.SetValue(std::move(t));
          } else {
            innerFuture._SetCallback(
                [prom = std::move(prom)](
                    typename TryWrapper<FReturnType>::Type &&t) mutable {
                  prom.SetValue(std::move(t));
                });
          }
        };

        if (sched)
          sched->Schedule(std::move(cb));
        else
          cb();
      });
    }

    return std::move(nextFuture);
  }

  /*
   * When register callbacks and timeout for a future like this:
   *      Future<int> f;
   *      f.Then(xx).OnTimeout(yy);
   *
   * There will be one future object created except f, we call f as root future.
   * The yy callback is registed on the last future, here are the possiblities:
   * 1. xx is called, and yy is not called.
   * 2. xx is not called, and yy is called.
   *
   * BUT BE CAREFUL BELOW:
   *
   *      Future<int> f;
   *      f.Then(xx).Then(yy).OnTimeout(zz);
   *
   * There will be 3 future objects created except f, we call f as root future.
   * The zz callback is registed on the last future, here are the possiblities:
   * 1. xx is called, and zz is called, yy is not called.
   * 2. xx and yy are called, and zz is called, aha, it's rarely happend but...
   * 3. xx and yy are called, it's the normal case.
   * So, you may shouldn't use OnTimeout with chained futures!!!
   */
  void OnTimeout(std::chrono::milliseconds duration, TimeoutCallback f,
                 Scheduler *scheduler) {

    scheduler->ScheduleLater(
        duration, [state = state_, cb = std::move(f)]() mutable {
          std::unique_lock<std::mutex> guard(state->thenLock_);
          if (state->progress_ != Progress::None)
            return;

          state->progress_ = Progress::Timeout;
          guard.unlock();

          cb();
        });
  }

private:
  void
  _SetCallback(std::function<void(typename TryWrapper<T>::Type &&)> &&func) {
    state_->then_ = std::move(func);
  }

  void _SetOnTimeout(std::function<void(TimeoutCallback &&)> &&func) {
    state_->onTimeout_ = std::move(func);
  }

  std::shared_ptr<State<T>> state_;
};
