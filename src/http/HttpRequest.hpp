/**
 * @file HttpRequest.hpp
 * @author JDongChen
 * @brief 请求协议
 * @version 0.1
 * @date 2022-08-14
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <memory>

enum class HttpMethod {

};

enum class HttpStatus {

};

enum class HttpContentType {

};

class HttpRequest {
private:
  HttpMethod method_;
  uint8_t version_;
  bool active_close_;
  bool websocket_;
  std::string path_;
  std::string query_;
  std::string fragment_;
  std::string body_;

  // TODO
public:
  HttpRequest(uint8_t version = 0x11, bool close = true);
};