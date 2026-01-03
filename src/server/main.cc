#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <optional>
#include <print>
#include <ranges>
#include <span>
#include <string>
#include <unordered_set>
#include <utility>

#include "aliasing.h"
#include "logic/hand_evaluator.h"
#include "model/card.h"
#include "model/deck.h"
#include "utility/card_serializer.h"
#include "utility/sorted_vector.h"

int main() {
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

  for (int a = 0; a < 10; a++) {
    std::chrono::time_point start = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000000; i++) {
      logic::HandEvaluator evaluator;
      logic::CombinationType combination =
          evaluator.Evaluate(hand.underlying());
    }
    std::chrono::time_point end = std::chrono::steady_clock::now();
    std::print(
        "Elapsed: {}\n",
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count());
  }

  // std::print("combination type {}\n", static_cast<u8>(combination));

  return 0;
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
