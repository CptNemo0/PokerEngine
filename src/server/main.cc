#include <optional>
#include <print>
#include <span>
#include <string>

#include "aliasing.h"
#include "model/card.h"
#include "model/deck.h"
#include "utility/card_serializer.h"
#include "utility/sorted_vector.h"

class HandEvaluator {
  public:
    void Evaluate(std::span<model::Card> hand) {
    }
};

int main() {
  model::Deck deck;

  model::Card card1{model::Card::Suit::kSpades, model::Card::Rank::kSix};
  model::Card card2{model::Card::Suit::kSpades, model::Card::Rank::kTwo};
  model::Card card3{model::Card::Suit::kSpades, model::Card::Rank::kFive};
  model::Card card4{model::Card::Suit::kSpades, model::Card::Rank::kFour};
  model::Card card5{model::Card::Suit::kSpades, model::Card::Rank::kThree};
  model::Card card6{model::Card::Suit::kSpades, model::Card::Rank::kSeven};

  sorted_vector<model::Card> hand;
  hand.insert(card1);
  hand.insert(card2);
  hand.insert(card3);
  hand.insert(card4);
  hand.insert(card5);
  hand.insert(card6);

  for (const auto& element : hand.data()) {
    std::print("{} |", utility::CardSerializer::Serialize(element));
  }

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
