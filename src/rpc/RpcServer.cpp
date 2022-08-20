#include "RpcServer.hpp"

// void RpcServer::start() {}

Serializer RpcServer::call(const std::string &name, const std::string &arg) {
  Serializer serializer;
  if (handlers_.find(name) == handlers_.end()) {
    return serializer;
  }

  auto fun = handlers_[name];
  fun(serializer, arg);

  return serializer;
}

std::shared_ptr<Protocol>
RpcServer::handleMethodCall(std::shared_ptr<Protocol> request) {
  std::string func_name;
  Serializer req(request->getContent());
  req >> func_name;
  Serializer rt = call(func_name, req.toString());
  auto response = Protocol::Create(Protocol::MsgType::RPC_METHOD_RESPONSE,
                                   rt.toString(), request->getSequenceId());
  return response;
}

void RpcServer::makeNewConnection(int connfd, const sockaddr_in &peer) {
  auto loop = _getNextLoop();
  auto func = [loop, connfd, peer, this]() {
    auto conn(std::make_shared<RpcSession>(loop));
    auto handleMethodCallFunc =
        std::bind(&RpcServer::handleMethodCall, this, std::placeholders::_1);
    conn->Init(connfd, peer);
    conn->sethandleMethodCall(handleMethodCallFunc);
    loop->Register(EPOLL_ET_Read, conn);
  };
  loop->RunInThisLoop(func);
}