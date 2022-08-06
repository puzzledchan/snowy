
#include "TimerManager.hpp"
#include <cassert>

// 清除超时结点，执行回调函数
template <typename E> void HeapTimerManager<E>::Tick() {
  if (heap_.empty()) {
    return;
  }
  while (!heap_.empty()) {
    auto node = heap_.front();
    if (std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() >
        0) {
      break;
    }
    node.cb(); // 回调
    Pop();
  }
}
// 获取最近的事件时间
template <typename E> int HeapTimerManager<E>::GetNextTick() {
  Tick();
  size_t res = -1;
  if (!heap_.empty()) {
    res = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now())
              .count();
    if (res < 0) {
      res = 0;
    }
  }
  return res;
}

void HeapTimerManager::Pop() {
  assert(!heap_.empty());
  _del(0);
}