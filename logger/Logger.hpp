/**
 * @file logger.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-08-01
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <stdarg.h>

#include <fstream>
#include <sstream>

#include <list>
#include <map>
#include <string>
#include <vector>

#include <memory>
#include <mutex>

#include "Singleton.hpp"
#include "SystemCall.hpp"

//颜色宏定义
#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define LIGHT_RED "\033[1;31m"
#define GREEN "\033[0;32;32m"
#define LIGHT_GREEN "\033[1;32m"
#define BLUE "\033[0;32;34m"
#define LIGHT_BLUE "\033[1;34m"
#define DARY_GRAY "\033[1;30m"
#define CYAN "\033[0;36m"
#define LIGHT_CYAN "\033[1;36m"
#define PURPLE "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN "\033[0;33m"
#define YELLOW "\033[1;33m"
#define LIGHT_GRAY "\033[0;37m"
#define WHITE "\033[1;37m"

/*固定日志级别输出日志内容*/
#define SNOWY_LOG_LEVEL(logger, level)                                         \
  if (logger->getLevel() <= level)                                             \
  LogEventWrap(std::shared_ptr<LogEvent>(new LogEvent(logger, level, __FILE__, \
                                                      __LINE__, GetThreadId(), \
                                                      time(0))))               \
      .getContentStream()

#define SNOWY_LOG_DEBUG(logger) SNOWY_LOG_LEVEL(logger, LogLevel::DEBUG)
#define SNOWY_LOG_INFO(logger) SNOWY_LOG_LEVEL(logger, LogLevel::INFO)
#define SNOWY_LOG_WARN(logger) SNOWY_LOG_LEVEL(logger, LogLevel::WARN)
#define SNOWY_LOG_ERROR(logger) SNOWY_LOG_LEVEL(logger, LogLevel::ERROR)
#define SNOWY_LOG_FATAL(logger) SNOWY_LOG_LEVEL(logger, LogLevel::FATAL)

/*带参日志输出日志内容*/
#define SNOWY_LOG_FMT_LEVEL(logger, level, fmt, ...)                           \
  if (level >= logger->getLevel())                                             \
  LogEventWrap(std::shared_ptr<LogEvent>(new LogEvent(logger, level, __FILE__, \
                                                      __LINE__, GetThreadId(), \
                                                      time(0))))               \
      .getEvent()                                                              \
      ->format(fmt, __VA_ARGS__)

#define SNOWY_LOG_FMT_DEBUG(logger, fmt, ...)                                  \
  SNOWY_LOG_FMT_LEVEL(logger, LogLevel::DEBUG, fmt, __VA_ARGS__)
#define SNOWY_LOG_FMT_INFO(logger, fmt, ...)                                   \
  SNOWY_LOG_FMT_LEVEL(logger, LogLevel::INFO, fmt, __VA_ARGS__)
#define SNOWY_LOG_FMT_WARN(logger, fmt, ...)                                   \
  SNOWY_LOG_FMT_LEVEL(logger, LogLevel::WARN, fmt, __VA_ARGS__)
#define SNOWY_LOG_FMT_ERROR(logger, fmt, ...)                                  \
  SNOWY_LOG_FMT_LEVEL(logger, LogLevel::ERROR, fmt, __VA_ARGS__)
#define SNOWY_LOG_FMT_FATAL(logger, fmt, ...)                                  \
  SNOWY_LOG_FMT_LEVEL(logger, LogLevel::FATAL, fmt, __VA_ARGS__)

//通过单例LoggerMgr--->访问到LogManager中默认的Logger实例化对象
#define SNOWY_LOG_ROOT() LogMgr::GetInstance()->getRoot()

//通过日志器的名字获取日志器实体
#define SNOWY_LOG_NAME(name) LogMgr::GetInstance()->getLogger(name)

class Logger;
class LogManager;

class LogLevel {
public:
  //日志级别
  enum Level {
    UNKNOW = 0, //未知信息
    DEBUG = 1,  //调试信息
    INFO = 2,   //一般信息
    WARN = 3,   //警告信息
    ERROR = 4,  //一般错误
    FATAL = 5   //致命错误
  };

  static const char *ToString(LogLevel::Level level);
  static LogLevel::Level FromString(const std::string &val);
};

class LogEvent {
private:
  const char *file_ = nullptr;     //输出文件
  int32_t line_ = 0;               //行号
  uint32_t thread_id_ = 0;         //线程ID
  uint64_t time_stamp_;            //时间戳
  std::stringstream content_;      // 输出流式内容
  std::shared_ptr<Logger> logger_; //所属日志器
  LogLevel::Level level_;          //日志等级

public:
  LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,
           const char *file, int32_t line, uint32_t thread_id, uint64_t time);

  const char *getFileName() const { return file_; };
  int32_t getLine() const { return line_; }
  uint32_t getThreadID() const { return thread_id_; }
  uint64_t getTime() const { return time_stamp_; }
  std::string getContent() const { return content_.str(); }
  std::stringstream &getContentStream() { return content_; }
  std::shared_ptr<Logger> getLogger() const { return logger_; }
  LogLevel::Level getLevel() const { return level_; }

public:
  /**
   * @brief 自定义日志模板格式
   *
   * @param fmt 传入的具体日志模板
   * @param ... 可变参数传参
   */
  void format(const char *fmt, ...);
  void format(const char *fmt, va_list al);
};

class LogEventWrap {
private:
  std::shared_ptr<LogEvent> event_;
  std::shared_ptr<LogManager> lmr_;

public:
  LogEventWrap(const std::shared_ptr<LogEvent> event);
  ~LogEventWrap();

  std::stringstream &getContentStream();
  std::shared_ptr<LogEvent> getEvent();
};

class LogFormatter {
private:
  std::string pattern_;
  bool error_{false};
  void init(); //解析模板，在创建时启动

public:
  LogFormatter(const std::string &pattern);
  const std::string &getPattern() const { return pattern_; }
  bool isError() const { return error_; }

public:
  std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level,
                     std::shared_ptr<LogEvent> event);
  class FormatItem {
  public:
    FormatItem(const std::string &fmt = "") {}
    virtual ~FormatItem() {}
    virtual void format(std::ostream &os, std::shared_ptr<Logger> logger,
                        LogLevel::Level level,
                        std::shared_ptr<LogEvent> event) = 0;
  };

private:
  std::vector<std::shared_ptr<FormatItem>> formatItems_;
};

class LogAppender {
protected:
  LogLevel::Level level_;
  std::shared_ptr<LogFormatter> formatter_;
  std::mutex append_mutex_;      //互斥锁
  bool is_set_formatter_{false}; //是否设置过日志格式

public:
  virtual ~LogAppender() {}
  virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
                   std::shared_ptr<LogEvent> event) = 0;
  LogLevel::Level getLevel() const { return level_; }
  bool getIsFormatter() { return is_set_formatter_; }
  std::shared_ptr<LogFormatter> getFormatter();

public:
  void setLevel(LogLevel::Level level) { level_ = level; }
  void setFormatter(std::shared_ptr<LogFormatter> val);
};

class Logger : public std::enable_shared_from_this<Logger> {
private:
  std::string name_;                                  // 日志器名称
  LogLevel::Level level_;                             // 日志等级
  std::list<std::shared_ptr<LogAppender>> appenders_; // 输出器指针队列
  std::mutex log_mutex_;                              // 互斥锁
  std::shared_ptr<LogFormatter> formatter_;           // 自带形式化输出
  std::shared_ptr<Logger> default_root;               // 默认root

public:
  Logger(const std::string &name = "root");
  const std::string &getName() const { return name_; }
  void setDefaultRoot(std::shared_ptr<Logger> logger) { default_root = logger; }

  LogLevel::Level getLevel() const { return level_; }
  void setLevel(LogLevel::Level level) { level_ = level; }

  std::shared_ptr<LogFormatter> getFormatter();
  void setFormatter(const std::string &val);

public:
  void addAppender(std::shared_ptr<LogAppender> appender);
  void delAppender(std::shared_ptr<LogAppender> appender);
  void clearAppender();

public:
  void log(LogLevel::Level level, const std::shared_ptr<LogEvent> event);
  void debug(std::shared_ptr<LogEvent> event);
  void info(std::shared_ptr<LogEvent> event);
  void warn(std::shared_ptr<LogEvent> event);
  void error(std::shared_ptr<LogEvent> event);
  void fatal(std::shared_ptr<LogEvent> event);
};

class LogManager {
private:
  //建立名字和日志器的映射关系
  std::map<std::string, std::shared_ptr<Logger>> s_loggers;
  //默认日志器
  std::shared_ptr<Logger> root_;
  //互斥锁
  std::mutex lmr_mutex_;

public:
  LogManager();
  std::shared_ptr<Logger> getLogger(const std::string &name);
  std::shared_ptr<Logger> getRoot() const { return root_; }
};

using LogMgr = Singleton<LogManager>;

class StdoutLogAppender : public LogAppender {
public:
  StdoutLogAppender();
  void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
           std::shared_ptr<LogEvent> event) override;
};
class FileLogAppender : public LogAppender {
public:
  FileLogAppender(const std::string &filename);
  void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
           std::shared_ptr<LogEvent> event) override;
  bool reopen();

private:
  //输出文件路径
  std::string file_name_;
  //输出的文件流
  std::ofstream file_stream_;
  //上一次打开文件的时间
  uint64_t last_time_ = 0;

  // uint64_t count_time = 0;
  struct timeval tv_begin;
  struct timeval tv_cur;
  uint64_t m_size = 0;
};