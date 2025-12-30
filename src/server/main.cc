#include <optional>
#include <print>
#include <string>

#include "aliasing.h"
#include "card.h"
#include "card_serializer.h"
#include "deck.h"

int main() {
  model::Deck deck;

  u32 i = 0;

  while (const std::optional<model::Card> card = deck.Deal()) {
    const std::string card_serialized =
        utility::CardSerializer::Serialize(card.value());
    const std::optional<model::Card> card_deserialized =
        utility::CardSerializer::Deserialize(card_serialized);
    if (card_deserialized.has_value()) {
      std::print("Card {} : {} | card == card_deserialized: {}\n", i++,
                 card_serialized, card.value() == card_deserialized.value());
    }
  }

  return 0;
}
