#include <chrono>
#include <functional>

class Scheduler {
public:
  virtual ~Scheduler() {}
  virtual void ScheduleLater(std::chrono::milliseconds duration,
                             std::function<void()> f) = 0;
  virtual void Schedule(std::function<void()> f) = 0;
};