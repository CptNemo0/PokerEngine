#include <cstdint>
#include <exception>
#include <format>
#include <iostream>
#include <memory>
#include <optional>
#include <print>
#include <stdexcept>
#include <string>

#include "ixwebsocket/IXConnectionState.h"
#include "ixwebsocket/IXWebSocket.h"
#include "ixwebsocket/IXWebSocketMessage.h"
#include "ixwebsocket/IXWebSocketMessageType.h"
#include "ixwebsocket/IXWebSocketServer.h"
#include <ixwebsocket/IXNetSystem.h>
#include <string_view>

#include "aliasing.h"
#include "logic/hand_evaluator.h"
#include "model/card.h"
#include "model/deck.h"
#include "net/net_init_manager.h"
#include "utility/card_serializer.h"
#include "utility/sorted_vector.h"

// Lobby should have a scheduler == croupier

namespace {

constexpr std::string_view host = "127.0.0.1";
constexpr int port = 8008;

} // namespace

class Server {
  public:
    Server(int port, const std::string_view& host)
        : server_(port, host.data()) {
    }

    void Start() {
      server_.setOnConnectionCallback(
          [&](std::weak_ptr<ix::WebSocket> webSocket,
              std::shared_ptr<ix::ConnectionState> connectionState) {
            std::print("New connection: {}\n", connectionState->getRemoteIp());
            auto ws = webSocket.lock();
            if (!ws) {
              return;
            }

            MessageHandler handler;
            handler.connection_state = connectionState;
            handler.server = this;
            handler.web_socket = webSocket;

            ws->setOnMessageCallback(handler);
          });

      const auto& [result, error] = server_.listen();
      if (!result) {
        throw std::logic_error(std::format("Could not connect: {}\n", error));
      }
      server_.start();
    }

    void Wait() {
      server_.wait();
    }

    void Stop() {
      server_.stop();
    }

  private:
    struct MessageHandler {
        std::weak_ptr<ix::WebSocket> web_socket;
        std::shared_ptr<ix::ConnectionState> connection_state;
        Server* server;

        void operator()(const ix::WebSocketMessagePtr& msg) {
          using WebSocketMessageType = ix::WebSocketMessageType;
          switch (msg->type) {
          case WebSocketMessageType::Open:
            std::print("New connection\nId: {}\nURI: {}\n",
                       connection_state->getId(), msg->openInfo.uri);
            std::cout << "Headers:" << std::endl;
            for (auto it : msg->openInfo.headers) {
              std::cout << it.first << ": " << it.second << std::endl;
            }
            break;

          case WebSocketMessageType::Close:
            std::print("Closing connection [{}].\nReason: {}\nCode: {}\n",
                       connection_state->getId(), msg->closeInfo.reason,
                       msg->closeInfo.code);
            break;

          case WebSocketMessageType::Error:
            std::print("ERROR: {}", msg->errorInfo.reason);
            break;

          case WebSocketMessageType::Message:
            std::print("Message from [{}]: {}\n", connection_state->getId(),
                       msg->str);
            break;

          case WebSocketMessageType::Fragment:
            std::print("Fragment\n");
            break;

          case WebSocketMessageType::Ping:
            std::print("Ping\n");
            break;

          case WebSocketMessageType::Pong:
            std::print("Pong\n");
            break;
          }
        }
    };

    ix::WebSocketServer server_;
};

int main() {
  net::NetInitManager::Initialize();

  Server server{port, host};
  server.Start();
  char a = ' ';
  while (a != 'q') {
    std::cin >> a;
  }
  server.Stop();

  return 0;

  model::Deck deck;

  model::Card card1{model::Card::Suit::kHearts, model::Card::Rank::kTen};
  model::Card card2{model::Card::Suit::kHearts, model::Card::Rank::kJack};
  model::Card card3{model::Card::Suit::kHearts, model::Card::Rank::kQueen};
  model::Card card4{model::Card::Suit::kHearts, model::Card::Rank::kKing};
  model::Card card5{model::Card::Suit::kHearts, model::Card::Rank::kAce};

  sorted_vector<model::Card> hand;
  hand.insert(card1);
  hand.insert(card2);
  hand.insert(card3);
  hand.insert(card4);
  hand.insert(card5);

  u32 i = 0;
  while (const std::optional<model::Card> card = deck.Deal()) {
    const std::string card_serialized =
        utility::CardSerializer::Serialize(card.value());
    const std::optional<model::Card> card_deserialized =
        utility::CardSerializer::Deserialize(card_serialized);
    if (card_deserialized.has_value()) {
      std::print("Card {} : {} | card == card_deserialized: {}\n", i++,
                 card_serialized,
                 card.value().value() == card_deserialized.value().value());
    }
  }

  return 0;
}
