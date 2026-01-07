#include "hand_evaluator.h"

#include "model/card.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <optional>
#include <ranges>
#include <span>

namespace model {

CombinationType HandEvaluator::Evaluate(std::span<const Card> hand) {
  static constexpr std::array<check_function, 9> checker_functions = {
      &HandEvaluator::CheckRoyalFlush,   &HandEvaluator::CheckStraightFlush,
      &HandEvaluator::CheckFourOfAKind,  &HandEvaluator::CheckFullHouse,
      &HandEvaluator::CheckFlush,        &HandEvaluator::CheckStraight,
      &HandEvaluator::CheckThreeOfAKind, &HandEvaluator::CheckTwoPair,
      &HandEvaluator::CheckPair,
  };

  for (std::size_t i{}; i < suit_count_mapping_.size(); i++) {
    suit_count_mapping_[i] = 0;
  }

  for (std::size_t i{}; i < rank_count_mapping_.size(); i++) {
    rank_count_mapping_[i] = 0;
  }

  for (const Card& card : hand) {
    suit_count_mapping_[card.suit()]++;
    rank_count_mapping_[card.rank()]++;
  }

  for (check_function function_pointer : checker_functions) {
    check_return_type result = (this->*function_pointer)(hand);
    if (result) {
      return result.value();
    }
  }

  return CombinationType::kHighCard;
}

HandEvaluator::check_return_type
HandEvaluator::CheckRoyalFlush(hand_type hand) const {
  const std::optional<Suit> potential_flush_suit =
      suit_count_mapping_.return_enum_for([](u8 value) {
        return value > 4;
      });

  // Acquiring a royal flush is impossible is none of the suits appears more
  // than 4 times in the hand.
  if (!potential_flush_suit) {
    return {};
  }

  auto contains = [&](Rank rank) {
    return std::find(hand.begin(), hand.end(),
                     Card{potential_flush_suit.value(), rank}) != hand.end();
  };

  return contains(Rank::kAce) && contains(Rank::kKing) &&
                 contains(Rank::kQueen) && contains(Rank::kJack) &&
                 contains(Rank::kTen)
             ? CombinationType::kRoyalFlush
             : check_return_type{};
}

HandEvaluator::check_return_type
HandEvaluator::CheckStraightFlush(hand_type hand) const {
  // Find potential flush suit. If one does not exist, return false and
  // uninitialized card.
  const std::optional<Suit> potential_flush_suit =
      suit_count_mapping_.return_enum_for([](u8 value) {
        return value > 4;
      });

  // Acquiring a straight flush is impossible is none of the suits appears more
  // than 4 times in the hand.
  if (!potential_flush_suit) {
    return {};
  }

  // If potential flush exist find the longest consecutive sequence of
  // Cards.
  auto filtered = hand | std::ranges::views::filter([&](const Card& card) {
                    return card.suit() == potential_flush_suit;
                  });

  auto iterator = filtered.cbegin();
  Rank previous_rank = iterator->rank();
  iterator++;

  u8 max_consecutive_counter = 0;
  // When the consecutive streak is greater or equal than 4 assign new value
  // to `streak_ending_card`. Counter starts from 0, so by the time it's at
  // 4, 5 consecutive numbers must have been seen.
  u8 consecutive_counter = 0;
  Card streak_ending_card;

  for (; iterator != filtered.end(); iterator++) {
    if (static_cast<i8>(iterator->rank()) ==
        static_cast<i8>(previous_rank) + 1) {
      consecutive_counter++;
      max_consecutive_counter =
          std::max(max_consecutive_counter, consecutive_counter);
    } else {
      consecutive_counter = 0;
    }

    if (max_consecutive_counter >= 4) {
      streak_ending_card = Card{potential_flush_suit.value(), iterator->rank()};
    }
    previous_rank = iterator->rank();
  }

  const bool result = max_consecutive_counter >= 4;

  return CombinationType::kStraightFlush;
}

HandEvaluator::check_return_type
HandEvaluator::CheckFourOfAKind([[maybe_unused]] hand_type hand) const {
  return rank_count_mapping_.return_enum_for([](u8 value) {
    return value == 4;
  })
             ? CombinationType::kFourOfAKind
             : check_return_type{};
}

HandEvaluator::check_return_type
HandEvaluator::CheckFullHouse(hand_type hand) const {
  check_return_type pair_result = CheckPair(hand);
  check_return_type triplet_result = CheckThreeOfAKind(hand);

  return pair_result && triplet_result ? CombinationType::kFullHouse
                                       : check_return_type{};
}

HandEvaluator::check_return_type
HandEvaluator::CheckFlush(hand_type hand) const {
  return suit_count_mapping_.return_enum_for([](u8 value) {
    return value == 5;
  })
             ? CombinationType::kFlush
             : check_return_type{};
}

HandEvaluator::check_return_type
HandEvaluator::CheckStraight(hand_type hand) const {
  u8 consecutive_counter = 0;
  u8 max_consecutive_counter = 0;
  Rank previous_rank = hand[0].rank();

  for (std::size_t i{1}; i < hand.size(); i++) {
    if (static_cast<i8>(hand[i].rank()) == static_cast<i8>(previous_rank) + 1) {
      consecutive_counter++;
      max_consecutive_counter =
          std::max(max_consecutive_counter, consecutive_counter);
    } else {
      consecutive_counter = 0;
    }

    previous_rank = hand[i].rank();
  }

  return max_consecutive_counter >= 4 ? CombinationType::kStraight
                                      : check_return_type{};
}

HandEvaluator::check_return_type
HandEvaluator::CheckThreeOfAKind([[maybe_unused]] hand_type hand) const {
  return rank_count_mapping_.return_enum_for([](u8 value) {
    return value == 3;
  })
             ? CombinationType::kFourOfAKind
             : check_return_type{};
}

HandEvaluator::check_return_type
HandEvaluator::CheckTwoPair([[maybe_unused]] hand_type hand) const {
  // Check if there is at least one pair.
  const std::optional<Rank> potential_rank_1 =
      rank_count_mapping_.return_enum_for([](u8 value) {
        return value == 2;
      });

  // Return if there isn't at least one pair.
  if (!potential_rank_1) {
    return {};
  }

  // Try to find another pair.
  const u8 first_pair_rank = static_cast<u8>(potential_rank_1.value());
  return rank_count_mapping_.return_enum_for([first_pair_rank](u8 value) {
    // Rank of the fairs pair should be omitted.
    if (value == first_pair_rank) {
      return false;
    }
    return value == 2;
  })
             ? CombinationType::kTwoPair
             : check_return_type{};
}

HandEvaluator::check_return_type
HandEvaluator::CheckPair([[maybe_unused]] hand_type hand) const {

  return rank_count_mapping_.return_enum_for([](u8 value) {
    return value == 2;
  })
             ? CombinationType::kPair
             : check_return_type{};
}

} // namespace model
