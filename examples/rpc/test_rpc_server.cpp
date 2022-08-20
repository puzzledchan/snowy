#include "RpcServer.hpp"

int add(int a, int b) { return a + b; }
std::string getStr() { return "hello world"; }
std::string CatString(std::vector<std::string> v) {
  std::string res;
  for (auto &s : v) {
    res += s;
  }
  return res;
}
int main(int argc, char **argv) {

  std::shared_ptr<RpcServer> server(new RpcServer);
  std::string str = "lambda";
  // acid::Address::ptr address = acid::Address::LookupAny("127.0.0.1:8081");
  server->registerMethod("add", add);
  server->registerMethod("getStr", getStr);
  server->registerMethod("CatString", CatString);
  server->registerMethod("sleep", [] { sleep(2); });

  server->Start();
}