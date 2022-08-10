/**
 * @file log.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-07-30
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SNOWY_LOG_H
#define SNOWY_LOG_H

#include "Buffer.hpp"
#include "Singleton.hpp"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

enum class LogLevel {
  logNone = 0x00,
  logINFO = 0x01 << 0,
  logDEBUG = 0x01 << 1,
  logWARN = 0x01 << 2,
  logERROR = 0x01 << 3,
  logUSR = 0x01 << 4,
  logALL = 0xFFFF,
};

enum class LogDest {
  logNone = 0x00,
  logConsole = 0x01 << 0,
  logFile = 0x01 << 1,
  logSocket = 0x01 << 2,
};

enum class LogColor {
  Red = 1,
  Green,
  Yellow,
  Normal,
  Blue,
  Purple,
  White,
  Max,
};

class Logger {
private:
  LogLevel level_;
  LogDest dest_;
  std::string directory_;
  std::string fileName_;
  struct BufferInfo {
    std::atomic<bool> using_;
    Buffer buffer_;
  };
  std::map<std::thread::id, std::unique_ptr<BufferInfo>> buffers_;
  std::mutex mutex_;
  bool shutdown_;

public:
  Logger();
  ~Logger(){};

  /*  Logger(const Logger &) = delete;
   void operator=(const Logger &) = delete;
   Logger(Logger &&) = delete;
   void operator=(Logger &&) = delete; */

public:
  bool Init(LogLevel level = LogLevel::logDEBUG,
            LogDest dest = LogDest::logConsole, const char *pDir = 0);

  void Shutdown();
  bool Update();
  Logger &SetCurLevel(LogLevel level);
  void Flush(LogLevel level);
  bool IsLevelForbid(LogLevel level) const {
    std::cout << "Level: " << std::endl;
    if (static_cast<int>(level) < static_cast<int>(level_)) {
      return true;
    }
    return false;
  }

public:
  Logger &operator<<(const char *msg);
  Logger &operator<<(const unsigned char *msg);
  Logger &operator<<(const std::string &msg);
  Logger &operator<<(void *);
  Logger &operator<<(unsigned char a);
  Logger &operator<<(char a);
  Logger &operator<<(unsigned short a);
  Logger &operator<<(short a);
  Logger &operator<<(unsigned int a);
  Logger &operator<<(int a);
  Logger &operator<<(unsigned long a);
  Logger &operator<<(long a);
  Logger &operator<<(unsigned long long a);
  Logger &operator<<(long long a);
  Logger &operator<<(double a);

private:
  std::size_t _Log(const char *data, std::size_t len);
  void _WriteLog(LogLevel level, std::size_t nLen, const char *data);
  void _Color(LogColor color);
  void _Reset() { curLevel_ = LogLevel::logNone; }

private:
  static const size_t MaxCharPerLog_ = 2048;
  // parallel format log string
  static thread_local char tmpBuffer_[MaxCharPerLog_];
  static thread_local std::size_t pos_;
  static thread_local int64_t lastLogSecond_;
  static thread_local int64_t lastLogMSecond_;
  static thread_local LogLevel curLevel_;
  static thread_local char tid_[16];
  static thread_local int tidLen_;
  static unsigned int seq_;
};

class LogManager {
private:
  std::thread iothread_;
  bool shutdown_;
  std::set<Logger *> busyLogs_;
  Logger nullLog_;
  LogLevel level_;

public:
  LogManager() : shutdown_(true) { nullLog_.Init(LogLevel::logNone); }
  void Start();
  void Stop();
  void AddBusyLog(Logger *){};
  std::shared_ptr<Logger> CreateLog(LogLevel level, LogDest dest,
                                    const char *dir = nullptr);
  Logger *NullLog() {
    std::cout << "NullLog: " << std::endl;
    return &nullLog_;
  }

private:
  void Run();
  std::mutex logsMutex_;
  std::vector<std::shared_ptr<Logger>> logs_;

  std::mutex mgrMutex_;
  std::condition_variable cond_;
};

/**
 * @brief 借助运算符重载，构建右值刷新
 *
 */
class LogHelper {
public:
  LogHelper(LogLevel level) : level_(level) {}
  Logger &operator=(Logger &log) {
    std::cout << "Flush: " << std::endl;
    log.Flush(level_);
    return log;
  }

private:
  LogLevel level_;
};

using LogMgr = SingletonPtr<LogManager>;

#define SNOW_LOG_DBG(log)                                                      \
  (!(log) || (log)->IsLevelForbid(LogLevel::logDEBUG))                         \
      ? LogMgr::GetInstance()->NullLog()                                       \
      : *LogHelper(LogLevel::logDEBUG) = log->SetCurLevel(LogLevel::logDEBUG)
#define SNOW_LOG_INF(log)                                                      \
  (!(log) || (log)->IsLevelForbid(LogLevel::logINFO))                          \
      ? LogMgr::GetInstance()->NullLog()                                       \
      : *LogHelper(LogLevel::logINFO) = log->SetCurLevel(LogLevel::logINFO)

#define SNOW_LOG_WRN(log)                                                      \
  (!(log) || (log)->IsLevelForbid(LogLevel::logWARN))                          \
      ? LogMgr::GetInstance()->NullLog()                                       \
      : *LogHelper(LogLevel::logWARN) = log->SetCurLevel(LogLevel::logWARN)

#define SNOW_LOG_ERR(log)                                                      \
  (!(log) || (log)->IsLevelForbid(LogLevel::logERR))                           \
      ? LogMgr::GetInstance()->NullLog()                                       \
      : *LogHelper(LogLevel::logERR) = log->SetCurLevel(LogLevel::logERR)

#define SNOW_LOG_USR(log)                                                      \
  (!(log) || (log)->IsLevelForbid(LogLevel::logUSR))                           \
      ? LogMgr::GetInstance()->NullLog()                                       \
      : *LogHelper(LogLevel::logUSR) = log->SetCurLevel(LogLevel::logUSR)

#define DBG SNOW_LOG_DBG
#define INF SNOW_LOG_INF
#define WRN SNOW_LOG_WRN
#define ERR SNOW_LOG_ERR
#define USR SNOW_LOG_USR

#endif