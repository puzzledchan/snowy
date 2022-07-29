

#include <chrono>
#include <functional>

#include <map>
#include <unordered_map>

using MS = std::chrono::milliseconds;
using Clock = std::chrono::high_resolution_clock;
using TimeStamp = Clock::time_point;

using TimeoutCallBack = std::function<void()>;

class TimerNode {
public:
  int id;
  TimeStamp expires;
  TimeoutCallBack cb;
  bool operator<(const TimerNode &other) const {
    return expires < other.expires;
  }
};

class TimerManager {};

template <typename E>

class HeapTimerManager : public TimerManager {
private:
  std::vector<TimerNode> heap_;
  std::unordered_map<E, size_t> ref_;

public:
  HeapTimerManager();

  void Tick();

  int GetNextTick();

public:
  void AdjustTimer();
  void AddTimer(int id, int timeout, const TimeoutCallBack &cb);
  void Pop();

private:
  void _clear();

  void _del(size_t index);
  void _siftup(size_t index);
  void _siftdown(size_t index);
  void _swapNode(size_t i, size_t j);
};