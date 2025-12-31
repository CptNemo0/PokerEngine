#ifndef COMMON_MODEL_CARD_H_
#define COMMON_MODEL_CARD_H_

#include "aliasing.h"

namespace model {

constexpr u32 gSuitNumber = 4;
constexpr u32 gRankNumber = 13;

// `Card` is a main class in the data model of the application - it models a
// singular playing card.
class Card {
  public:
    enum class Suit : i8 {
      kNone = -1,
      kMinValue = 0,
      kSpades = kMinValue,
      kClubs = 1,
      kDiamonds = 2,
      kHearts = 3,
      kMaxValue = kHearts,
    };

    enum class Rank : i8 {
      kNone = -1,
      kMinValue = 0,
      kTwo = kMinValue,
      kThree = 1,
      kFour = 2,
      kFive = 3,
      kSix = 4,
      kSeven = 5,
      kEighth = 6,
      kNine = 7,
      kTen = 8,
      kJack = 9,
      kQueen = 10,
      kKing = 11,
      kAce = 12,
      kMaxValue = kAce,
    };

    Card() = default;
    Card(Card::Suit suit, Card::Rank rank);
    Card(const Card& other);
    Card(Card&& other) noexcept;

    void operator=(const Card& other);
    void operator=(Card&& other) noexcept;

    bool operator==(const Card& other) const {
      return other.suit() == suit() && other.rank() == rank();
    }
    bool operator>(const Card& other) const;
    bool operator<(const Card& other) const;

    Card::Suit suit() const {
      return suit_;
    }

    Card::Rank rank() const {
      return rank_;
    }

    u32 value() const;

  private:
    Suit suit_{Card::Suit::kNone};
    Rank rank_{Card::Rank::kNone};
};

} // namespace model

#endif // !COMMON_MODEL_CARD_H_
