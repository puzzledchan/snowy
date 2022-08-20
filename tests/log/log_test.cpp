#include "log.hpp"

int main(int argc, char **argv) {

  LogMgr::GetInstance()->Start();
  auto log =
      LogMgr::GetInstance()->CreateLog(LogLevel::logDEBUG, LogDest::logConsole);

  DBG(log) << 10
           << "|abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijk"
              "lmnopqrstuvwxy|";
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  LogMgr::GetInstance()->Stop();
}