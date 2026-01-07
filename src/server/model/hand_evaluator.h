#ifndef SERVER_LOGIC_HAND_EVALUATOR_H_
#define SERVER_LOGIC_HAND_EVALUATOR_H_

#include <optional>
#include <span>

#include "aliasing.h"
#include "model/card.h"
#include "utility/enum_indexable_array.h"

namespace model {

enum class CombinationType : i8 {
  kRoyalFlush = 0,
  kStraightFlush = 1,
  kFourOfAKind = 2,
  kFullHouse = 3,
  kFlush = 4,
  kStraight = 5,
  kThreeOfAKind = 6,
  kTwoPair = 7,
  kPair = 8,
  kHighCard = 9
};

class HandEvaluator {
  public:
    using check_return_type = std::optional<CombinationType>;
    using hand_type = std::span<const Card>;
    using check_function =
        check_return_type (HandEvaluator::*)(hand_type) const;

    // Evaluates the hand. Returns the highest scoring combination that can be
    // made out of the hand. If unable to obtain any combination it defaults to
    // returning "kHighCard".
    CombinationType Evaluate(std::span<const Card> hand);

  private:
    using Rank = Card::Rank;
    using Suit = Card::Suit;

    // Royal flush contains cards 10 J Q K A of the same suit.
    check_return_type CheckRoyalFlush(hand_type hand) const;

    // Straight flush contains consecutive cards of the same suit.
    check_return_type CheckStraightFlush(hand_type hand) const;

    // Four of a kind contains four cards of the same rank.
    check_return_type CheckFourOfAKind([[maybe_unused]] hand_type hand) const;

    // Full house contains a pair and a triplet (three of a kind).
    check_return_type CheckFullHouse(hand_type hand) const;

    // Flush contains 5 cards of the same suit in no particular order.
    check_return_type CheckFlush(hand_type hand) const;

    // Straight is contains as 5 cards with consecutive ranks.
    check_return_type CheckStraight(hand_type hand) const;

    // Three of a kind contains three cards of the same rank.
    check_return_type CheckThreeOfAKind([[maybe_unused]] hand_type hand) const;

    // Two pairs contains two pairs of cards of the same rank.
    check_return_type CheckTwoPair([[maybe_unused]] hand_type hand) const;

    // Pair contains two cards of the same rank.
    check_return_type CheckPair([[maybe_unused]] hand_type hand) const;

    // Mapping: Suit - Number of time it appears in the hand.
    common::utility::enum_indexable_array<Suit, u8, 4> suit_count_mapping_;

    // Mapping: Rank - Number of time it appears in the hand.
    common::utility::enum_indexable_array<Rank, u8, 13> rank_count_mapping_;
};

} // namespace model

#endif // !SERVER_LOGIC_HAND_EVALUATOR_H_
