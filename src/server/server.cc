#include "server.h"

#include "connection_closure_handler.h"
#include "ixwebsocket/IXConnectionState.h"
#include "ixwebsocket/IXWebSocket.h"
#include "ixwebsocket/IXWebSocketMessage.h"
#include "ixwebsocket/IXWebSocketMessageType.h"
#include "ixwebsocket/IXWebSocketServer.h"

#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <format>
#include <memory>
#include <mutex>
#include <print>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "lobby.h"
#include "server_manager.h"
#include "stacktrace_analyzer.h"

namespace server {

Server::Server(int port, const std::string_view& host, Lobby& lobby,
               ConnectionClosureHandler& closure_handler)
  : server_(std::make_unique<ix::WebSocketServer>(port, host.data())),
    lobby_(lobby), closure_handler_(closure_handler),
    server_manager_observation_(this) {
  server_manager_observation_.Observe(
    std::addressof(ServerManager::Instance()));
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
        MessageHandler{webSocket, connectionState, this});
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
    // std::print("New connection\nId: {}\nURI: {}\nHeaders:\n", state->getId(),
    //            msg->openInfo.uri);
    // for (auto it : msg->openInfo.headers) {
    //   std::print("{}: {}\n", it.first, it.second);
    // }
    if (!server) {
      return;
    }
    server->OnNewConnectionEstablished(web_socket, state);

    break;

  case MessageType::Close:
    std::print("Closing connection [{}].\nReason: {}\nCode: {}\n",
               state->getId(), msg->closeInfo.reason, msg->closeInfo.code);
    if (state && server) {
      server->OnConnectionClosed(*state);
    }
    break;

  case MessageType::Error:
    std::print("ERROR: {}", msg->errorInfo.reason);
    break;

  case MessageType::Message:
    // server->OnMessageReceived(msg->str, connection);
    // std::print("Message from [{}]: {}\n", connection.state->getId(),
    // msg->str);
    break;

  case MessageType::Fragment:
  case MessageType::Ping:
  case MessageType::Pong:
    break;
  }
}

void Server::End() {
  std::print("Server...");
  stop_ = true;
  // for (auto& connection : connections_) {
  //   connection->web_socket.lock()->close();
  // }
  server_->stop();
  std::print("finished\n");
}

void Server::OnNewConnectionEstablished(
  std::weak_ptr<ix::WebSocket> web_socket,
  std::shared_ptr<ix::ConnectionState> state) {
  if (stop_) {
    return;
  }
  std::shared_ptr<Connection> connection =
    std::make_shared<Connection>(web_socket, state);
  {
    std::lock_guard lock{connections_mutex_};
    // connections_.push_back(connection);
  }
  lobby_.Push(std::move(connection));
}

void Server::OnConnectionClosed(const ix::ConnectionState& state) {
  if (stop_) {
    return;
  }
  const std::string& sid = state.getId();
  u64 id = 0;
  std::from_chars(sid.data(), sid.data() + sid.size(), id, 10);

  auto find_lambda = [id](const std::shared_ptr<Connection> connection) {
    return connection->id == id;
  };

  std::lock_guard lock{connections_mutex_};

  auto find_result = std::ranges::find_if(connections_, find_lambda);
  if (find_result == connections_.end()) {
    std::print("Closed connection was not in `connections_` "
               "collection. You're cooked!!!\n");
    common::utility::StacktraceAnalyzer::PrintOut();
    // std::abort();
  }
  (*find_result)->closed.store(true);
  std::print("Connection {} erased from connections_\n", (*find_result)->id);
  connections_.erase(find_result);
  closure_handler_.OnConnectionClosed(id);
}

} // namespace server
