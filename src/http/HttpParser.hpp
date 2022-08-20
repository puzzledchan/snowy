/**
 * @file HttpParser.hpp
 * @author JDongChen
 * @brief HTTP 协议解析抽象类
 * @version 0.1
 * @date 2022-08-14
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "HttpRequest.hpp"
#include "coroutine.hpp"

class HttpParser {
public:
  /// 错误码
  enum Error {
    NO_ERROR = 0,
    INVALID_METHOD,
    INVALID_PATH,
    INVALID_VERSION,
    INVALID_LINE,
    INVALID_HEADER,
    INVALID_CODE,
    INVALID_REASON,
    INVALID_CHUNK
  };
  enum CheckState { NO_CHECK, CHECK_LINE, CHECK_HEADER, CHECK_CHUNK };

protected:
  char *cur_ = nullptr;

  Error error_ = NO_ERROR;
  CheckState checkState_ = NO_CHECK;

public:
  HttpParser() {}
  virtual ~HttpParser();

  size_t execute(char *data, size_t len, bool chunk = false);
  int isFinished();
  int hasError();
  void setError(Error v) { error_ = v; }

protected:
protected:
  virtual Task<Error> parse_line() = 0;
  virtual Task<Error> parse_header() = 0;
  virtual Task<Error> parse_chunk() = 0;
};

/**
 * @brief HTTP请求解析
 *
 */
// TODO:
class HttpRequest;
class HttpRequestParser : public HttpParser {
private:
  std::shared_ptr<HttpRequest> request_;

public:
  HttpRequestParser() { request_ = std::make_shared<HttpRequest>(); }

public:
  std::shared_ptr<HttpRequest> getRequest() { return request_; }

private:
  void on_request_method(const std::string &str);

  void on_request_path(const std::string &str);

  void on_request_query(const std::string &str);

  void on_request_fragment(const std::string &str);

  void on_request_version(const std::string &str);

  void on_request_header(const std::string &key, const std::string &val);

  void on_request_header_done();

protected:
  Task<Error> parse_line() override;

  Task<Error> parse_header() override;

  Task<Error> parse_chunk() override;
};