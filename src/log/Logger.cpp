/**
 * @file Logger.cpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-08-01
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <algorithm>
#include <functional>

#include <iostream>
#include <string.h>

#include "Logger.hpp"

const char *LogLevel::ToString(LogLevel::Level level) {
  switch (level) {
#define LevelToString(name)                                                    \
  case LogLevel::name:                                                         \
    return #name;                                                              \
    break;

    LevelToString(DEBUG);
    LevelToString(INFO);
    LevelToString(WARN);
    LevelToString(FATAL);
    LevelToString(ERROR);
#undef LevelToString
  default:
    return "UNKNOWN";
  }
  return "UNKNOW";
}

LogLevel::Level LogLevel::FromString(const std::string &val) {
#define StringToLevel(name)                                                    \
  if (strcasecmp(val.c_str(), #name) == 0) {                                   \
    return LogLevel::name;                                                     \
  }
  // strcasecmp 解决小写不识别
  StringToLevel(DEBUG);
  StringToLevel(INFO);
  StringToLevel(WARN);
  StringToLevel(FATAL);
  StringToLevel(ERROR);
#undef StringToLevel
  return LogLevel::UNKNOW;
}

/*****************************FormatItem***************************************/
//输出颜色格式
class ColorFormatItem : public LogFormatter::FormatItem {
public:
  ColorFormatItem(const std::string &str = "") {}
  void format(std::ostream &os, std::shared_ptr<Logger> logger,
              LogLevel::Level level, std::shared_ptr<LogEvent> event) override {
    switch (level) {
    case LogLevel::UNKNOW:
      os << DARY_GRAY;
      break;
    case LogLevel::DEBUG:
      os << GREEN;
      break;
    case LogLevel::INFO:
      os << WHITE;
      break;
    case LogLevel::WARN:
      os << YELLOW;
      break;
    case LogLevel::ERROR:
      os << LIGHT_RED;
      break;
    case LogLevel::FATAL:
      os << RED;
      break;
    default:
      break;
    }
  }
};

//输出日志消息内容
class MessageFormatItem : public LogFormatter::FormatItem {
public:
  MessageFormatItem(const std::string &str = "") {}
  void format(std::ostream &os, std::shared_ptr<Logger> logger,
              LogLevel::Level level, std::shared_ptr<LogEvent> event) override {
    os << event->getContent();
  }
};

//输出日志级别
class LevelFormatItem : public LogFormatter::FormatItem {
public:
  LevelFormatItem(const std::string &str = "") {}
  void format(std::ostream &os, std::shared_ptr<Logger> logger,
              LogLevel::Level level, std::shared_ptr<LogEvent> event) override {
    os << LogLevel::ToString(level);
  }
};

// 输出日志名称
class LogNameFormatItem : public LogFormatter::FormatItem {
public:
  LogNameFormatItem(const std::string &str = "") {}
  void format(std::ostream &os, std::shared_ptr<Logger> logger,
              LogLevel::Level level, std::shared_ptr<LogEvent> event) override {
    // os << logger->getName();
    //改动: 打印者Logger 可能是default_root 不能直接如上面使用getName()
    os << event->getLogger()->getName();
  }
};

// 输出线程名称
class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
  ThreadIdFormatItem(const std::string &str = "") {}
  void format(std::ostream &os, std::shared_ptr<Logger> logger,
              LogLevel::Level level, std::shared_ptr<LogEvent> event) override {
    os << event->getThreadID();
  }
};

// 输出日期格式
class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
  DateTimeFormatItem(const std::string &format = "%Y-%m-%d %H:%M:%S")
      : format_(format) {
    if (!format_.size())
      format_ = "%Y-%m-%d %H:%M:%S";
  }

  void format(std::ostream &os, std::shared_ptr<Logger> logger,
              LogLevel::Level level, std::shared_ptr<LogEvent> event) override {
    time_t t = (time_t)(event->getTime());
    struct tm tm;
    char s[100];
    tm = *localtime(&t);
    strftime(s, sizeof(s), format_.c_str(), &tm);
    os << s;
  }

private:
  std::string format_; //时间显示格式
};

// 输出行号
class LineFormatItem : public LogFormatter::FormatItem {
public:
  LineFormatItem(const std::string &str = "") {}

  void format(std::ostream &os, std::shared_ptr<Logger> logger,
              LogLevel::Level level, std::shared_ptr<LogEvent> event) override {
    os << event->getLine();
  }
};

// 输出换行符
class NewLineFormatItem : public LogFormatter::FormatItem {
public:
  NewLineFormatItem(const std::string &str = "") {}

  void format(std::ostream &os, std::shared_ptr<Logger> logger,
              LogLevel::Level level, std::shared_ptr<LogEvent> event) override {
    os << std::endl;
  }
};

// 输出文件名
class FileNameFormatItem : public LogFormatter::FormatItem {
public:
  FileNameFormatItem(const std::string &str = "") {}

  void format(std::ostream &os, std::shared_ptr<Logger> logger,
              LogLevel::Level level, std::shared_ptr<LogEvent> event) override {
    os << event->getFileName();
  }
};

class StringFormatItem : public LogFormatter::FormatItem {
public:
  StringFormatItem(const std::string &m_s) : string_(m_s) {}

  void format(std::ostream &os, std::shared_ptr<Logger> logger,
              LogLevel::Level level, std::shared_ptr<LogEvent> event) override {
    os << string_;
  }

private:
  std::string string_; //文本内容
};

// 输出Tab键
class TabFormatItem : public LogFormatter::FormatItem {
public:
  TabFormatItem(const std::string &str = "") {}

  void format(std::ostream &os, std::shared_ptr<Logger> logger,
              LogLevel::Level level, std::shared_ptr<LogEvent> event) override {
    os << "\t";
  }
};

/*****************************LogEvent***************************************/
LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,
                   const char *file, int32_t line, uint32_t thread_id,
                   uint64_t time)
    : file_(file), line_(line), thread_id_(thread_id), time_stamp_(time),
      logger_(logger), level_(level) {}

//做一个可变参数列表
void LogEvent::format(const char *fmt, ...) {
  va_list al;
  va_start(al, fmt);
  format(fmt, al);
  va_end(al);
}

//传入已经准备好的可变参数 进行一个组合的得到的结果放在buf中
//并且构造一个string对象以"流"的方式送入到stringstream中去作为日志内容
void LogEvent::format(const char *fmt, va_list al) {
  char *buf = nullptr;
  int len = vasprintf(&buf, fmt, al);
  // vasprintf会动态分配内存 失败返回-1
  if (len != -1) {
    content_ << std::string(buf, len);
    free(buf);
  }
}

/***************************LogEventWrap**************************************************************/

LogEventWrap::LogEventWrap(std::shared_ptr<LogEvent> event) : event_(event) {}

LogEventWrap::~LogEventWrap() {
  //通过日志属性对象中日志器去打印日志内容
  event_->getLogger()->log(event_->getLevel(), event_);
}
std::stringstream &LogEventWrap::getContentStream() {
  return event_->getContentStream();
}
std::shared_ptr<LogEvent> LogEventWrap::getEvent() { return event_; }

/***********************************Logger********************************/
Logger::Logger(const std::string &name) : name_(name), level_(LogLevel::DEBUG) {
  //在生成Logger时候会生成一个默认的LogFormatter
  // 颜色 [logger-level:] <文件> 线程 时间 换行
  formatter_.reset(
      new LogFormatter("%c[%g-%p:]%T<%f:%l>%T%t%T%d{%Y-%m-%d %H:%M:%S}%T%m%n"));
}
//添加日志输出器
void Logger::addAppender(std::shared_ptr<LogAppender> appender) {
  std::lock_guard<std::mutex> guard(log_mutex_);
  //发现加入的日志输出器没有格式器的话 赋予默认格式器
  if (!appender->getFormatter()) {
    //此处造成死锁了
    //锁 LogAppender中的LogFormatter
    appender->setFormatter(formatter_);
  }
  appenders_.push_back(appender);
}
//删除日志输出器
void Logger::delAppender(std::shared_ptr<LogAppender> appender) {
  std::lock_guard<std::mutex> guard(log_mutex_);
  auto it = find(appenders_.begin(), appenders_.end(), appender);
  appenders_.erase(it);
}

void Logger::clearAppender() {
  std::lock_guard<std::mutex> guard(log_mutex_);
  appenders_.clear();
}
//设置日志格式器
void Logger::setFormatter(const std::string &val) {
  //解析模板并创建新的日志格式器
  std::shared_ptr<LogFormatter> formatter = std::make_shared<LogFormatter>(val);
  //模板解析是否有错误
  if (formatter->isError()) {
    std::cout << "Logger setFormatter name=" << name_ << "value=" << val
              << "invaild formatter" << std::endl;

    return;
  }

  std::lock_guard<std::mutex> guard(log_mutex_);
  formatter_ = formatter;

  //将所有LogAppender 没有设置过格式的formatter和Logger同步
  for (auto &appender : appenders_) {
    if (!appender->getIsFormatter())
      appender->setFormatter(formatter);
  }
}
//获取日志格式器
std::shared_ptr<LogFormatter> Logger::getFormatter() {
  std::lock_guard<std::mutex> guard(log_mutex_);
  return formatter_;
}

//打印日志 最终有LogAppender下的子类中的log去完成
void Logger::log(LogLevel::Level level, const std::shared_ptr<LogEvent> event) {
  //如果传入的日志级别 >= 当前日志器的级别都能进行输出
  if (level >= level_) {
    //拿到this指针的智能指针
    auto self = shared_from_this();
    if (appenders_.size()) {
      //操作日志输出器的时候要独占 锁住
      std::lock_guard<std::mutex> guard(log_mutex_);
      for (auto &appender : appenders_) {
        //调用的是appender中的log()进行实际输出
        appender->log(self, level, event);
      }
    } else if (default_root) {
      //是一层递归  使用default_root中的日志输出器来打印
      default_root->log(level, event);
    }
  }
}

void Logger::debug(std::shared_ptr<LogEvent> event) {
  log(LogLevel::DEBUG, event);
}
void Logger::info(std::shared_ptr<LogEvent> event) {
  log(LogLevel::INFO, event);
}
void Logger::warn(std::shared_ptr<LogEvent> event) {
  log(LogLevel::WARN, event);
}
void Logger::error(std::shared_ptr<LogEvent> event) {
  log(LogLevel::ERROR, event);
}
void Logger::fatal(std::shared_ptr<LogEvent> event) {
  log(LogLevel::FATAL, event);
}

/**********************LogAppender**********************/
//多线程环境下 可能别的线程在读 而当前要写
void LogAppender::setFormatter(std::shared_ptr<LogFormatter> val) {
  std::lock_guard<std::mutex> guard(append_mutex_);
  formatter_ = val;
  //传入的日志格式器不为nullptr 该输出器被设置过格式
  if (formatter_)
    is_set_formatter_ = true;
  else
    is_set_formatter_ = false;
}

//多线程环境下 可能别的线程在写 而当前要读
std::shared_ptr<LogFormatter> LogAppender::getFormatter() {
  std::lock_guard<std::mutex> guard(append_mutex_);
  return formatter_;
}

StdoutLogAppender::StdoutLogAppender() {
  formatter_.reset(
      new LogFormatter("%c[%g-%p:]%T<%f:%l>%T%t%T%d{%Y-%m-%d %H:%M:%S}%T%m%n"));
}

void StdoutLogAppender::log(std::shared_ptr<Logger> logger,
                            LogLevel::Level level,
                            std::shared_ptr<LogEvent> event) {
  if (level >= level_) {
    // std::cout 不是线程安全的
    std::lock_guard<std::mutex> guard(append_mutex_);
    std::cout << formatter_->format(logger, level, event);
  }
}

FileLogAppender::FileLogAppender(const std::string &filename)
    : file_name_(filename) {
  // count_time = time(0);
  //初始化时间结构体
  formatter_.reset(
      new LogFormatter("[%g-%p:]%T<%f:%l>%T%t%T%d{%Y-%m-%d %H:%M:%S}%T%m%n"));

  memset(&tv_cur, 0, sizeof(struct timeval));
  // gettimeofday(&tv_cur, nullptr);
  //将当前文件重打开一次
  reopen();
}
//文件重打开
bool FileLogAppender::reopen() {
  //锁 文件句柄
  std::lock_guard<std::mutex> guard(append_mutex_);
  if (file_stream_) {
    file_stream_.close();
  }
  //以追加方式打开文件
  file_stream_.open(file_name_, std::ios::app);
  return !file_stream_;
}

//文件存储日志信息
void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level,
                          std::shared_ptr<LogEvent> event) {
  if (level >= level_) {

    uint64_t now = time(0);

    if (now != last_time_) //这个!=不等于意味着 每过1s就reopen一次
    {
      reopen();
      last_time_ = now;
    }

    std::lock_guard<std::mutex> guard(append_mutex_);
    // gettimeofday(&tv_begin, nullptr);
    std::string t = formatter_->format(logger, level, event);
    file_stream_ << t;
  }
}

/**********************LogFormatter**********************/
LogFormatter::LogFormatter(const std::string &pattern) : pattern_(pattern) {
  //解析传入模板格式
  init();
}

//解析模板 套用模板
void LogFormatter::init() {
  // 1.模板字符串 2.普通字符串 3.是否为正确有效模板
  std::vector<std::tuple<std::string, std::string, int>> mv;
  //缓存和模板内容无关的普通字符串 一并加入到mv中
  std::string str = "";

  for (size_t i = 0; i < pattern_.size(); i++) {
    //遇到"%"前的普通字符存入到str中并且跳到下一个字符
    if (pattern_[i] != '%') {
      str += pattern_[i];
      continue;
    }
    //连续遇到两个"%%" 进行转义处理
    if (i + 1 < pattern_.size() && pattern_[i + 1] == '%') {
      str += '%';
      continue;
    }
    std::string pcahr = ""; //暂时存储相关模板代表字母
    std::string fmt = "";   //存储"{...}"间的模板内容
    size_t index = i + 1;   // '%'后的第一个位置
    int fmt_status = 0;     //有限状态机表示
    size_t fmt_begin = 0;   // "{...}"间内容的起点

    while (index < pattern_.size()) {
      // 碰到符号H:非字符 && 非'{' && 非'}'
      // 后将"%XXXH"紧跟串XXX存入temp中，并且break
      if (fmt_status == 0 &&
          (!isalpha(pattern_[index]) && pattern_[index] != '{' &&
           pattern_[index] != '}')) {
        pcahr = pattern_.substr(i + 1, index - i - 1);
        break;
      }

      // fmt_status表示一种有限状态机,处理括号中的内容
      // 0 = 未遇到 '{'
      // 1 = 已经遇到'{' , 未遇到'}'
      if (fmt_status == 0) {
        //%XXX{   存储XXX串
        if (pattern_[index] == '{') {
          pcahr = pattern_.substr(i + 1, index - i - 1);
          fmt_status = 1;
          fmt_begin = index;
          ++index;
          continue;
        }

      } else if (fmt_status == 1) {
        // {XXXX} 存储XXX串
        if (pattern_[index] == '}') {
          fmt = pattern_.substr(fmt_begin + 1, index - fmt_begin - 1);
          fmt_status = 0;
          ++index;
          break;
        }
      }

      ++index;

      // "%XXXXXX"  存储XXXXXX串
      if (index == pattern_.size()) {
        if (!pcahr.size()) {
          pcahr = pattern_.substr(i + 1);
        }
      }
    }

    //没有遇到 { 或者 完成了括号序列的扫描
    if (fmt_status == 0) {
      //%aaaaXXX%bbbb  或者  XXX%aaaa  存储XXX
      if (pcahr.size()) {
        mv.push_back(std::make_tuple(str, std::string(), 0));
        str.clear();
      }

      //%XXX 存储XXX 且 {YYY} 存储YYY
      mv.push_back(std::make_tuple(pcahr, fmt, 1));
      i = index - 1;
    }
    //有 { 但没有遇到 } 一定是一个错误的序列
    else if (fmt_status == 1) {
      std::cout << "pattern parse error" << pattern_ << "--"
                << pattern_.substr(i) << std::endl;

      error_ = true;

      mv.push_back(std::make_tuple("<<pattern error>>", fmt, 0));
    }
  }

  // "%aaaXXX"存储XXX 即最后尾部为普通字符串的情况
  if (str.size()) {
    mv.push_back(std::make_tuple(str, "", 0));
  }

  // %c-------颜色输入
  // %p-------level
  // %n-------换行符
  // %m-------日志内容
  // %%-------输出一个%
  // %t-------当前线程ID
  // %T-------Tab键
  // %d-------日期和时间
  // %f-------文件名
  // %l-------行号
  // %g-------日志器名字

  //将字符 和 对应的函数操作建立映射关系
  //可以使用函数指针完成，这里使用了函数包装器function + lambda表达式，非常方便
  static std::map<std::string, std::function<std::shared_ptr<FormatItem>(
                                   const std::string &str)>>
      s_format_items = {
#define CharToItem(str, C)                                                     \
  {                                                                            \
#str, [](const std::string                                                 \
                 &fmt) { return std::shared_ptr<FormatItem>(new C(fmt)); }     \
  }

          /*注意必须是 ',' */
          CharToItem(c, ColorFormatItem),
          CharToItem(m, MessageFormatItem),
          CharToItem(p, LevelFormatItem),
          CharToItem(t, ThreadIdFormatItem),
          CharToItem(d, DateTimeFormatItem),
          CharToItem(l, LineFormatItem),
          CharToItem(n, NewLineFormatItem),
          CharToItem(f, FileNameFormatItem),
          CharToItem(T, TabFormatItem),
          CharToItem(g, LogNameFormatItem)
#undef XX
      };

  for (auto &x : mv) {
    //元组中标志int = 0 说明不是模板内容 构造为普通字符串对象
    if (std::get<2>(x) == 0) {
      //构造输出文本内容的子类对象
      formatItems_.push_back(
          std::shared_ptr<FormatItem>(new StringFormatItem(std::get<0>(x))));
    } else {
      auto it = s_format_items.find(std::get<0>(x));
      //如果mv中 存储的<0>位置的字符串
      //无法与map中的映射关系对应,说明存储了一个错误的字符模板
      if (it == s_format_items.end()) {
        //构造输出文本内容的子类对象 输出一下错误
        formatItems_.push_back(std::shared_ptr<FormatItem>(
            new StringFormatItem("<<error_format %" + std::get<0>(x) + ">>")));

        error_ = true;
      } else {
        //给对应的子类构造传参<1>位置的fmt字符串
        formatItems_.push_back(it->second(std::get<1>(x)));
      }
    }
  }
}

// LogFormatter中的format  最终调用具体FormatItem中的format进行打印
std::string LogFormatter::format(std::shared_ptr<Logger> logger,
                                 LogLevel::Level level,
                                 std::shared_ptr<LogEvent> event) {
  std::stringstream ss;
  for (auto &formatItems : formatItems_) {
    formatItems->format(ss, logger, level, event);
  }

  return ss.str();
}

/*********************************LogManager**************************************************/

LogManager::LogManager() {
  //生成一个默认的Logger 并且配置一个默认的LogAppender 输出到控制台
  root_.reset(new Logger);
  root_->addAppender(std::shared_ptr<LogAppender>(new StdoutLogAppender));

  //存入到容器中
  s_loggers[root_->getName()] = root_;
}

std::shared_ptr<Logger> LogManager::getLogger(const std::string &name) {
  std::lock_guard<std::mutex> lock(lmr_mutex_);
  auto it = s_loggers.find(name);
  if (it != s_loggers.end())
    return it->second;

  auto logger = std::make_shared<Logger>(name);
  s_loggers[name] = logger;
  //给新创建的日志器Logger一个
  logger->setDefaultRoot(root_);
  return logger;
}
