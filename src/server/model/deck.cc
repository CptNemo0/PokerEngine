#include "model/deck.h"

#include <algorithm>
#include <array>
#include <numeric>
#include <optional>

#include "aliasing.h"
#include "model/card.h"

namespace model {

Deck::Deck() {
  const i32 minimum_rank = static_cast<i32>(Card::Rank::kMinValue);
  const i32 maximum_rank = static_cast<i32>(Card::Rank::kMaxValue);

  const i32 minimum_suit = static_cast<i32>(Card::Suit::kMinValue);
  const i32 maximum_suit = static_cast<i32>(Card::Suit::kMaxValue);

  for (i32 i = minimum_rank; i <= maximum_rank; i++) {
    for (i32 j = minimum_suit; j <= maximum_suit; j++) {
      cards_[j + i * gSuitNumber] =
          Card(static_cast<Card::Suit>(j), static_cast<Card::Rank>(i));
    }
  }

  Reshuffle();
}

void Deck::Reshuffle() {
  std::iota(shuffled_view_.begin(), shuffled_view_.end(), 0);
  std::shuffle(shuffled_view_.begin(), shuffled_view_.end(), generator_);
  deal_pointer_ = 0;
}

std::optional<Card> Deck::Deal() {
  return deal_pointer_ < gDeckSize ? cards_[shuffled_view_[deal_pointer_++]]
                                   : std::optional<Card>(std::nullopt);
}

} // namespace model
