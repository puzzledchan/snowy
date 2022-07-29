
#include <iostream>
#include <type_traits>

int fun(int x) { return x + 1; }

int main() {
  std::result_of<decltype (&fun)(int)>::type d = 10;
  std::cout << d << std::endl;
  return 0;
}

// d的类型是fun返回类型int