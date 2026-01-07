#include "server.h"

#include "ixwebsocket/IXConnectionState.h"
#include "ixwebsocket/IXWebSocket.h"
#include "ixwebsocket/IXWebSocketMessage.h"
#include "ixwebsocket/IXWebSocketMessageType.h"
#include "ixwebsocket/IXWebSocketServer.h"

#include <format>
#include <memory>
#include <print>
#include <stdexcept>
#include <string_view>

#include "lobby.h"

namespace server {

Server::Server(int port, const std::string_view& host, Lobby& lobby)
  : server_(std::make_unique<ix::WebSocketServer>(port, host.data())),
    lobby_(lobby) {
}

void Server::Start() {
  server_->setOnConnectionCallback(
    [&](std::weak_ptr<ix::WebSocket> webSocket,
        std::shared_ptr<ix::ConnectionState> connectionState) {
      std::print("New connection: {}\n", connectionState->getRemoteIp());
      auto ws = webSocket.lock();
      if (!ws) {
        return;
      }

      ws->setOnMessageCallback(
        MessageHandler{{webSocket, connectionState}, this});
    });

  const auto& [result, error] = server_->listen();
  if (!result) {
    throw std::logic_error(std::format("Could not connect: {}\n", error));
  }
  server_->start();
}

void Server::MessageHandler::operator()(const ix::WebSocketMessagePtr& msg) {
  using MessageType = ix::WebSocketMessageType;
  switch (msg->type) {
  case MessageType::Open:
    std::print("New connection\nId: {}\nURI: {}\nHeaders:\n",
               connection.state->getId(), msg->openInfo.uri);
    for (auto it : msg->openInfo.headers) {
      std::print("{}: {}\n", it.first, it.second);
    }
    if (!server) {
      return;
    }
    server->OnNewConnectionEstablished(connection);

    break;

  case MessageType::Close:
    std::print("Closing connection [{}].\nReason: {}\nCode: {}\n",
               connection.state->getId(), msg->closeInfo.reason,
               msg->closeInfo.code);
    server->OnConnectionClosed(connection);
    break;

  case MessageType::Error:
    std::print("ERROR: {}", msg->errorInfo.reason);
    break;

  case MessageType::Message:
    server->OnMessageReceived(msg->str, connection);
    std::print("Message from [{}]: {}\n", connection.state->getId(), msg->str);
    break;

  case MessageType::Fragment:
  case MessageType::Ping:
  case MessageType::Pong:
    break;
  }
}

void Server::OnNewConnectionEstablished(const Connection& connection) {
  lobby_.Push(connection);
}

} // namespace server
