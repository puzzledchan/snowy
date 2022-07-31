#include "Connection.hpp"
#include "TcpServer.hpp"
#include <atomic>
#include <unistd.h>

size_t OnMessage(Connection *conn, const char *data, size_t len) {
  // echo package
  std::string rsp(data, len);
  // conn->SendPacket(rsp.data(), rsp.size());
  return len;
}

void OnNewConnection(ananas::Connection *conn) {
  using ananas::Connection;

  conn->SetOnMessage(OnMessage);
  conn->SetOnDisconnect([](Connection *conn) {
    WRN(logger) << "OnDisConnect " << conn->Identifier();
  });
}

int main(int ac, char *av[]) {
  size_t workers = 1;
  if (ac > 1)
    workers = (size_t)std::stoi(av[1]);

  /*   ananas::LogManager::Instance().Start();
    logger = ananas::LogManager::Instance().CreateLog(logALL, logALL,
                                                      "logger_server_test"); */

  TcpServer server;
  server.Listen("127.0.0.1", 9987, OnNewConnection);

  server.Run(ac, av);

  return 0;
}
