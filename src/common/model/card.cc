#include "model/card.h"

#include <cassert>
#include <exception>
#include <utility>

#include "aliasing.h"

namespace model {

Card::Card(Card::Suit suit, Card::Rank rank) : suit_(suit), rank_(rank) {
}

Card::Card(const Card& other) : Card(other.suit(), other.rank()) {
}

Card::Card(Card&& other) noexcept
    : Card(std::exchange(other.suit_, Card::Suit::kNone),
           std::exchange(other.rank_, Card::Rank::kNone)) {
}

void Card::operator=(const Card& other) {
  suit_ = other.suit();
  rank_ = other.rank();
}

void Card::operator=(Card&& other) noexcept {
  suit_ = std::exchange(other.suit_, Card::Suit::kNone);
  rank_ = std::exchange(other.rank_, Card::Rank::kNone);
};

bool Card::operator>(const Card& other) const {
  if (rank_ == other.rank_)
    return suit_ > other.suit_;
  return rank_ > other.rank_;
};

bool Card::operator<(const Card& other) const {
  if (rank_ == other.rank_)
    return suit_ < other.suit_;
  return rank_ < other.rank_;
};

u32 Card::value() const {
  return static_cast<u32>(suit()) * gRankNumber + static_cast<u32>(rank());
}

} // namespace model
