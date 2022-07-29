/**
 * @file HttpParser.cpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-08-14
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "HttpParser.hpp"

void HttpRequestParser::on_request_method(const std::string &str) {}

Task<HttpParser::Error> HttpRequestParser::parse_line() {
  std::string buff;
  while (isalpha(*cur_)) {
    buff.push_back(*cur_);
    co_yield NO_ERROR;
  }

  if (buff.empty()) {
    co_return INVALID_METHOD;
  }
}