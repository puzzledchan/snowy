#include <bit>
#include <iostream>
enum class endian;
int main() {
  if constexpr (std::endian::native == std::endian::big)
    std::cout << "big-endian\n";
  else if constexpr (std::endian::native == std::endian::little)
    std::cout << "little-endian\n";
  else
    std::cout << "mixed-endian\n";
}