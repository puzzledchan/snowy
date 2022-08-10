/**
 * @file log.cpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-07-30
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "log.hpp"
#include "Timer.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>

Logger::Logger() : level_(LogLevel::logNone), dest_(LogDest::logNone) {}

bool Logger::Init(LogLevel level, LogDest dest, const char *pDir) {
  level_ = level;
  dest_ = dest;
  return true;
}

bool Logger::Update() {
  std::vector<std::unique_ptr<BufferInfo>> logBufs;
  bool todo = false;
  {
    std::unique_lock<std::mutex> guard(mutex_);
    for (auto iter = buffers_.begin(); iter != buffers_.end();) {
      assert(iter->second);
      if (iter->second->using_) {
        todo = true;
        iter++;
      } else {
        logBufs.push_back(std::move(iter->second));
        iter = buffers_.erase(iter);
      }
    }
  }
  for (auto &pbuf : logBufs) {
    const char *data = pbuf->buffer_.ReadArr();
    const auto size = pbuf->buffer_.ReadableSize();
    auto nWritten = _Log(data, size);
    assert(nWritten == size);
  }
  // file_.Sync(); 文件同步
  return todo;
}
void Logger::Shutdown() {
  std::unique_lock<std::mutex> guard(mutex_);
  if (shutdown_)
    return;
  shutdown_ = true;
}

static const size_t kPrefixLevelLen = 6;
static const size_t kPrefixTimeLen = 27;
thread_local char Logger::tmpBuffer_[Logger::MaxCharPerLog_];
thread_local std::size_t Logger::pos_ = kPrefixLevelLen + kPrefixTimeLen;
thread_local int64_t Logger::lastLogSecond_ = -1;
thread_local int64_t Logger::lastLogMSecond_ = -1;
thread_local LogLevel Logger::curLevel_ = LogLevel::logNone;
thread_local char Logger::tid_[16] = "";
thread_local int Logger::tidLen_ = 0;

unsigned int Logger::seq_ = 0;

static const int kFlushThreshold = 2 * 1024 * 1024;
/**
 * @brief 依据level刷新
 *
 * @param level
 */
void Logger::Flush(LogLevel level) {
  std::cout << "Flush " << std::endl;
  assert(level == curLevel_);
  if (IsLevelForbid(curLevel_)) {
    _Reset();
    return;
  }
  if (pos_ < kPrefixTimeLen + kPrefixLevelLen) {
    assert(false);
    return;
  }

  Time now;
  auto seconds = now.MilliSeconds() / 1000;
  if (seconds != lastLogSecond_) {
    now.FormatTime(tmpBuffer_);
    lastLogSecond_ = seconds;
  } else {
    auto msec = now.MicroSeconds() % 1000000;
    if (msec != lastLogMSecond_) {
      snprintf(tmpBuffer_ + 20, 7, "%06d", static_cast<int>(msec));
      tmpBuffer_[26] = ']';
      lastLogMSecond_ = msec;
    }
  }

  if (pos_ == kPrefixTimeLen + kPrefixLevelLen)
    return;
  switch (level_) {
  case LogLevel::logINFO:
    memcpy(tmpBuffer_ + kPrefixTimeLen, "[INF]:", kPrefixLevelLen);
    break;
  case LogLevel::logDEBUG:
    memcpy(tmpBuffer_ + kPrefixTimeLen, "[DBG]:", kPrefixLevelLen);
    break;
  case LogLevel::logWARN:
    memcpy(tmpBuffer_ + kPrefixTimeLen, "[WRN]:", kPrefixLevelLen);
    break;
  case LogLevel::logERROR:
    memcpy(tmpBuffer_ + kPrefixTimeLen, "[ERR]:", kPrefixLevelLen);
    break;
  case LogLevel::logUSR:
    memcpy(tmpBuffer_ + kPrefixTimeLen, "[USR]:", kPrefixLevelLen);
    break;
  }
  if (tidLen_ == 0) {
    std::ostringstream oss;
    oss << std::this_thread::get_id();

    const auto &str = oss.str();
    tidLen_ = std::min<int>(str.size(), sizeof tid_);
    tid_[0] = '|'; // | thread_id
    memcpy(tid_ + 1, str.data(), tidLen_);
    tidLen_ += 1;
  }

  memcpy(tmpBuffer_ + pos_, tid_, tidLen_);
  pos_ += tidLen_;

  tmpBuffer_[pos_++] = '\n';
  tmpBuffer_[pos_] = '\0';

  BufferInfo *info = nullptr;
  {
    std::cout << "BufferInfo " << std::endl;
    std::unique_lock<std::mutex> guard(mutex_);
    if (shutdown_) {
      std::cout << tmpBuffer_;
      return;
    }

    info = buffers_[std::this_thread::get_id()].get();
    if (!info)
      buffers_[std::this_thread::get_id()].reset(info = new BufferInfo());

    assert(!info->using_);
    info->using_ = true;
  }
  _Reset();
  // Format: level info, length, log msg
  int logLevel = static_cast<int>(level);

  info->buffer_.Append(&logLevel, sizeof logLevel);
  info->buffer_.Append(&pos_, sizeof pos_);
  info->buffer_.Append(tmpBuffer_, pos_);
  if (info->buffer_.ReadableSize() > kFlushThreshold) {
    info->using_ = false;
    LogMgr::GetInstance()->AddBusyLog(this);
  } else {
    info->using_ = false;
  }
}
/**
 * @brief 颜色设定
 *
 * @param color
 */
void Logger::_Color(LogColor color) {
  const int max = static_cast<int>(LogColor::Max);
  int index = static_cast<int>(color);
  const char *colorstrings[max] = {
      "",        "\033[1;31;40m", "\033[1;32;40m", "\033[1;33;40m",
      "\033[0m", "\033[1;34;40m", "\033[1;35;40m", "\033[1;37;40m",
  };

  fprintf(stdout, "%s", colorstrings[index]);
}

size_t Logger::_Log(const char *data, size_t dataLen) {
  std::cout << "_Log " << std::endl;
  const auto minLogSize = sizeof(int) + sizeof(size_t);
  size_t nOffset = 0;
  while (nOffset + minLogSize < dataLen) {
    int level = *(int *)(data + nOffset);
    size_t len = *(size_t *)(data + nOffset + sizeof(int));
    if (dataLen < nOffset + minLogSize + len) {
      std::cerr << "_WriteLog skip 0!!!\n ";
      break;
    }
    LogLevel clevel = static_cast<LogLevel>(level);
    std::cout << "clevel: " << level << std::endl;
    _WriteLog(clevel, len, data + nOffset + minLogSize);
    std::cout << "tttt: " << level << std::endl;
    nOffset += minLogSize + len;
  }
  return nOffset;
}

void Logger::_WriteLog(LogLevel level, size_t len, const char *data) {
  std::cout << "Write Log " << std::endl;
  assert(len > 0 && data);
  if (dest_ == LogDest::logConsole) {
    switch (level) {
    case LogLevel::logINFO:
      _Color(LogColor::Green);
      break;
    case LogLevel::logDEBUG:
      _Color(LogColor::White);
      break;
    case LogLevel::logWARN:
      _Color(LogColor::Yellow);
      break;
    case LogLevel::logERROR:
      _Color(LogColor::Red);
      break;
    case LogLevel::logUSR:
      _Color(LogColor::Purple);
      break;
    }
    fprintf(stdout, "%.*s", static_cast<int>(len), data);
    _Color(LogColor::Normal);

    // !TODO: logFile
    /*      if (dest_ & logFile) {
       while (_CheckChangeFile()) {
         _CloseLogFile();
         if (!_OpenLogFile(_MakeFileName().c_str()))
           break;
       }

       assert(file_.IsOpen());
       file_.Write(data, len);
     } */
  }
}
Logger &Logger::SetCurLevel(LogLevel level) {
  curLevel_ = level;
  return *this;
}

Logger &Logger::operator<<(const char *msg) {
  if (IsLevelForbid(curLevel_))
    return *this;

  const auto len = strlen(msg);
  if (pos_ + len >= MaxCharPerLog_)
    return *this;

  memcpy(tmpBuffer_ + pos_, msg, len);
  pos_ += len;

  return *this;
}

Logger &Logger::operator<<(const unsigned char *msg) {
  return operator<<(reinterpret_cast<const char *>(msg));
}

Logger &Logger::operator<<(const std::string &msg) {
  return operator<<(msg.c_str());
}

Logger &Logger::operator<<(void *ptr) {
  if (IsLevelForbid(curLevel_))
    return *this;

  if (pos_ + 18 < MaxCharPerLog_) {
    unsigned long ptrValue = (unsigned long)ptr;
    auto nbytes =
        snprintf(tmpBuffer_ + pos_, MaxCharPerLog_ - pos_, "%#018lx", ptrValue);
    if (nbytes > 0)
      pos_ += nbytes;
  }

  return *this;
}

Logger &Logger::operator<<(unsigned char a) {
  if (IsLevelForbid(curLevel_))
    return *this;

  if (pos_ + 3 < MaxCharPerLog_) {
    auto nbytes = snprintf(tmpBuffer_ + pos_, MaxCharPerLog_ - pos_, "%hhd", a);
    if (nbytes > 0)
      pos_ += nbytes;
  }

  return *this;
}

Logger &Logger::operator<<(char a) {
  if (IsLevelForbid(curLevel_))
    return *this;

  if (pos_ + 3 < MaxCharPerLog_) {
    auto nbytes = snprintf(tmpBuffer_ + pos_, MaxCharPerLog_ - pos_, "%hhu", a);
    if (nbytes > 0)
      pos_ += nbytes;
  }

  return *this;
}

Logger &Logger::operator<<(unsigned short a) {
  if (IsLevelForbid(curLevel_))
    return *this;

  if (pos_ + 5 < MaxCharPerLog_) {
    auto nbytes = snprintf(tmpBuffer_ + pos_, MaxCharPerLog_ - pos_, "%hu", a);
    if (nbytes > 0)
      pos_ += nbytes;
  }

  return *this;
}

Logger &Logger::operator<<(short a) {
  if (IsLevelForbid(curLevel_))
    return *this;

  if (pos_ + 5 < MaxCharPerLog_) {
    auto nbytes = snprintf(tmpBuffer_ + pos_, MaxCharPerLog_ - pos_, "%hd", a);
    if (nbytes > 0)
      pos_ += nbytes;
  }

  return *this;
}

Logger &Logger::operator<<(unsigned int a) {
  if (IsLevelForbid(curLevel_))
    return *this;

  if (pos_ + 10 < MaxCharPerLog_) {
    auto nbytes = snprintf(tmpBuffer_ + pos_, MaxCharPerLog_ - pos_, "%u", a);
    if (nbytes > 0)
      pos_ += nbytes;
  }

  return *this;
}

Logger &Logger::operator<<(int a) {
  if (IsLevelForbid(curLevel_))
    return *this;

  if (pos_ + 10 < MaxCharPerLog_) {
    auto nbytes = snprintf(tmpBuffer_ + pos_, MaxCharPerLog_ - pos_, "%d", a);
    if (nbytes > 0)
      pos_ += nbytes;
  }

  return *this;
}

Logger &Logger::operator<<(unsigned long a) {
  if (IsLevelForbid(curLevel_))
    return *this;

  if (pos_ + 20 < MaxCharPerLog_) {
    auto nbytes = snprintf(tmpBuffer_ + pos_, MaxCharPerLog_ - pos_, "%lu", a);
    if (nbytes > 0)
      pos_ += nbytes;
  }

  return *this;
}

Logger &Logger::operator<<(long a) {
  if (IsLevelForbid(curLevel_))
    return *this;

  if (pos_ + 20 < MaxCharPerLog_) {
    auto nbytes = snprintf(tmpBuffer_ + pos_, MaxCharPerLog_ - pos_, "%ld", a);
    if (nbytes > 0)
      pos_ += nbytes;
  }

  return *this;
}

Logger &Logger::operator<<(unsigned long long a) {
  if (IsLevelForbid(curLevel_))
    return *this;

  if (pos_ + 20 < MaxCharPerLog_) {
    auto nbytes = snprintf(tmpBuffer_ + pos_, MaxCharPerLog_ - pos_, "%llu", a);
    if (nbytes > 0)
      pos_ += nbytes;
  }

  return *this;
}

Logger &Logger::operator<<(long long a) {
  if (IsLevelForbid(curLevel_))
    return *this;

  if (pos_ + 20 < MaxCharPerLog_) {
    auto nbytes = snprintf(tmpBuffer_ + pos_, MaxCharPerLog_ - pos_, "%lld", a);
    if (nbytes > 0)
      pos_ += nbytes;
  }

  return *this;
}

Logger &Logger::operator<<(double a) {
  if (IsLevelForbid(curLevel_))
    return *this;

  if (pos_ + 20 < MaxCharPerLog_) {
    auto nbytes = snprintf(tmpBuffer_ + pos_, MaxCharPerLog_ - pos_, "%.6g", a);
    if (nbytes > 0)
      pos_ += nbytes;
  }

  return *this;
}

void LogManager::Start() {
  std::unique_lock<std::mutex> guard(mgrMutex_);
  assert(shutdown_);
  shutdown_ = false;

  auto io = std::bind(&LogManager::Run, this);
  iothread_ = std::thread{std::move(io)};
}

void LogManager::Run() {
  const std::chrono::milliseconds FlushInterval(1);
  bool running = true;
  while (running) {
    std::vector<Logger *> logsBusy;
    {
      std::unique_lock<std::mutex> guard(mgrMutex_);
      cond_.wait_for(guard, FlushInterval); // 等待唤醒或者超时解锁
      if (shutdown_) {
        running = false;
      }
    }

    if (logsBusy.empty()) {
      std::unique_lock<std::mutex> guard(logsMutex_);
      for (auto &plog : logs_)
        logsBusy.push_back(plog.get());
    }
    for (auto plog : logsBusy) {
      plog->Update();
    }
  }
}

void LogManager::Stop() {
  {
    std::unique_lock<std::mutex> guard(mgrMutex_);
    if (shutdown_) {
      return;
    }
    shutdown_ = true;
    guard.unlock();
    cond_.notify_all();
  }
  {
    std::lock_guard<std::mutex> guard(logsMutex_);
    for (auto &plog : logs_) {
      plog->Shutdown();
    }
  }
  if (iothread_.joinable()) {
    iothread_.join();
  }
}

std::shared_ptr<Logger> LogManager::CreateLog(LogLevel level, LogDest dest,
                                              const char *dir) {

  auto log(std::make_shared<Logger>());

  if (!log->Init(level, dest, dir)) {
    std::shared_ptr<Logger> nulllog(&nullLog_, [](Logger *) {});
    return nulllog;
  } else {
    std::lock_guard<std::mutex> guard(logsMutex_);
    if (shutdown_) {
      std::cerr << "Warning: Please call LogManager::Start() first\n";
      std::shared_ptr<Logger> nulllog(&nullLog_, [](Logger *) {});
      return nulllog;
    }

    logs_.emplace_back(log);
  }

  return log;
}