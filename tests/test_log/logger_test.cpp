#include <iostream>

#include "Logger.hpp"

using namespace std;

int main() {
  auto log = LogMgr::GetInstance()->getLogger("test");
  log->addAppender(
      std::shared_ptr<LogAppender>(new FileLogAppender("./test_log.log")));
  log->addAppender(std::shared_ptr<LogAppender>(new StdoutLogAppender));
  SNOWY_LOG_DEBUG(log) << "SNOWY_LOG_DEBUG";
  SNOWY_LOG_INFO(log) << "SNOWY_LOG_INFO";
  SNOWY_LOG_WARN(log) << "SNOWY_LOG_WARN";
  SNOWY_LOG_ERROR(log) << "SNOWY_LOG_ERROR";
  SNOWY_LOG_FATAL(log) << "SNOWY_LOG_FATAL";
  return 0;
}
