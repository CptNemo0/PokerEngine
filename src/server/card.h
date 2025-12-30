#ifndef SERVER_CARD_H_
#define SERVER_CARD_H_

#include "aliasing.h"

namespace model {

constexpr u32 gSuitNumber = 4;
constexpr u32 gRankNumber = 13;

class Card {
  public:
    enum class Suit {
      kNone = -1,
      kMinValue = 0,
      kSpades = kMinValue,
      kClubs = 1,
      kDiamonds = 2,
      kHearts = 3,
      kMaxValue = kHearts,
    };

    enum class Rank {
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
      kAce = 9,
      kJack = 10,
      kQueen = 11,
      kKing = 12,
      kMaxValue = kKing,
    };

    Card() = default;
    Card(Card::Suit suit, Card::Rank rank);

    Card::Suit suit() const {
      return suit_;
    }

    Card::Rank rank() const {
      return rank_;
    }

    bool operator==(const Card& other) const {
      return other.suit() == suit() && other.rank() == rank();
    }

  private:
    Suit suit_{Card::Suit::kNone};
    Rank rank_{Card::Rank::kNone};
};

} // namespace model

#endif // !SERVER_CARD_H_
