#include "Buffer.hpp"
#include "Logger.hpp"

#include <string>
#include <thread>
auto g_log = SNOWY_LOG_ROOT();

void test() {
  Buffer buffer;
  std::string buf("hello world");
  buffer.pushData(&*buf.begin(), buf.size());
  std::string dst(buf.size(), ' ');
  buffer.popData(&*dst.begin(), buf.size());
  SNOWY_LOG_DEBUG(g_log) << dst;
  SNOWY_LOG_DEBUG(g_log) << buffer.writableSize();
  SNOWY_LOG_DEBUG(g_log) << buffer.capacity();
}
int main() {
  test();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  return 0;
}