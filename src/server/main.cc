#include <algorithm>
#include <format>
#include <iostream>
#include <memory>
#include <optional>
#include <print>
#include <string>

#include <ixwebsocket/IXNetSystem.h>
#include <string_view>
#include <vector>

#include "aliasing.h"
#include "ixwebsocket/IXConnectionState.h"
#include "ixwebsocket/IXWebSocket.h"
#include "model/card.h"
#include "model/deck.h"
#include "model/hand_evaluator.h"
#include "net/net_init_manager.h"
#include "server.h"
#include "utility/card_serializer.h"
#include "utility/sorted_vector.h"
#include "utility/ts_queue.h"
// Lobby should have a scheduler == croupier

namespace {

constexpr std::string_view gHost = "127.0.0.1";
constexpr int gPort = 8008;

constexpr u8 gMaxPlayersInLobby = 3;

} // namespace

class MessageDispatcher {};

int main() {
  common::net::NetInitManager::Initialize();

  server::Server server{gPort, gHost};
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

  common::utility::sorted_vector<model::Card> hand;
  hand.insert(card1);
  hand.insert(card2);
  hand.insert(card3);
  hand.insert(card4);
  hand.insert(card5);

  u32 i = 0;
  while (const std::optional<model::Card> card = deck.Deal()) {
    const std::string card_serialized =
        common::utility::CardSerializer::Serialize(card.value());
    const std::optional<model::Card> card_deserialized =
        common::utility::CardSerializer::Deserialize(card_serialized);
    if (card_deserialized.has_value()) {
      std::print("Card {} : {} | card == card_deserialized: {}\n", i++,
                 card_serialized,
                 card.value().value() == card_deserialized.value().value());
    }
  }

  return 0;
}
