//
// Created by zavier on 2022/1/13.
//

#include "Logger.hpp"
#include "Serializer.hpp"

#include <iostream>

auto g_log = SNOWY_LOG_ROOT();
template <typename T> void test_map(T &a) {
  Serializer s;
  s << a;
  // s.reset();
  T b;
  s >> b;
  for (auto item : b) {
    SNOWY_LOG_DEBUG(g_log) << item.first << " "
                           << item.second; //<< item.second;
  }
}

void test6() {
  std::map<int, std::string> a{{1, "a"}, {2, "b"}};
  test_map(a);
}

void test7() {
  std::multimap<int, std::string> a{{1, "a"}, {1, "a"}};
  test_map(a);
}

void test8() {
  std::unordered_multimap<int, std::string> a{{1, "a"}, {1, "a"}};
  test_map(a);
}

void test9() {
  std::unordered_map<int, std::string> a{{1, "a"}, {1, "a"}};
  test_map(a);
}

void seq2seq() {
  std::vector<std::string> a{"ab", "cd"};
  Serializer s;
  s << a;
  std::list<std::string> b;
  // s.reset();
  s >> b;
}

int main() { test6(); }