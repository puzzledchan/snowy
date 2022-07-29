//
// Created by zavier on 2022/1/13.
//

#include "RpcClient.hpp"

void test_call() {
  std::shared_ptr<RpcClient> client(new RpcClient());
  client->start();
  int n = 0;
  while (n != 100) {
    // ACID_LOG_DEBUG(g_logger) << n++;
    n++;
    std::cout << "n:" << n << std::endl;
    auto rt = client->call<int>("add", -2, n);
    int i = rt.getVal();
    std::cout << "res:" << i << std::endl;
  }
}

int main() { test_call(); }
