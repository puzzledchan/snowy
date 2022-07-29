/**
 * @file test_singleton.cpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-07-30
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "Singleton.hpp"
#include <iostream>

class TestClass {
public:
  void echo() { std::cout << "echo()" << std::endl; }
  TestClass() {}

private:
};
int main(int argc, char **argv) {

  SingletonPtr<TestClass>::GetInstance()->echo();
  return 0;
}