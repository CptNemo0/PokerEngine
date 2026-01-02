#include <algorithm>
#include <array>
#include <cstddef>
#include <optional>
#include <print>
#include <ranges>
#include <span>
#include <string>
#include <unordered_set>
#include <utility>

#include "aliasing.h"
#include "enum_indexable_array.h"
#include "model/card.h"
#include "model/deck.h"
#include "utility/card_serializer.h"
#include "utility/sorted_vector.h"

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
    CombinationType Evaluate(std::span<const model::Card> hand) {

      static constexpr std::array<checker_function, 9> checker_functions = {
          &HandEvaluator::CheckRoyalFlush,   &HandEvaluator::CheckStraightFlush,
          &HandEvaluator::CheckFourOfAKind,  &HandEvaluator::CheckFullHouse,
          &HandEvaluator::CheckFlush,        &HandEvaluator::CheckStraight,
          &HandEvaluator::CheckThreeOfAKind, &HandEvaluator::CheckTwoPair,
          &HandEvaluator::CheckPair,
      };

      card_count_mapping_.clear();
      // suit_count_mapping_.clear();

      for (std::size_t i{}; i < suit_count_mapping_.size(); i++) {
        suit_count_mapping_[i] = 0;
      }

      for (std::size_t i{}; i < rank_count_mapping_.size(); i++) {
        rank_count_mapping_[i] = 0;
      }

      for (const model::Card& card : hand) {
        card_count_mapping_.insert(card);
        suit_count_mapping_[card.suit()]++;
        rank_count_mapping_[card.rank()]++;
      }

      for (checker_function function_pointer : checker_functions) {
        const auto& [result, type] = (this->*function_pointer)(hand);
        if (result) {
          return type;
        }
      }

      return CombinationType::kHighCard;
    }

  private:
    using return_type = std::pair<bool, CombinationType>;
    using hand_type = std::span<const model::Card>;
    using checker_function = return_type (HandEvaluator::*)(hand_type) const;

    // Royal flush contains cards 10 J Q K A of the same suit. First check
    // if any suit appears more than 4 times, than if those cards are
    // correct.
    return_type CheckRoyalFlush([[maybe_unused]] hand_type hand) const {
      const std::optional<model::Card::Suit> potential_flush_suit =
          suit_count_mapping_.return_enum_for([](u8 value) {
            return value > 4;
          });

      if (!potential_flush_suit.has_value()) {
        return {false, {}};
      }

      const bool result =
          card_count_mapping_.contains(
              {potential_flush_suit.value(), model::Card::Rank::kTen}) &&
          card_count_mapping_.contains(
              {potential_flush_suit.value(), model::Card::Rank::kJack}) &&
          card_count_mapping_.contains(
              {potential_flush_suit.value(), model::Card::Rank::kQueen}) &&
          card_count_mapping_.contains(
              {potential_flush_suit.value(), model::Card::Rank::kKing}) &&
          card_count_mapping_.contains(
              {potential_flush_suit.value(), model::Card::Rank::kAce});

      return {result, CombinationType::kRoyalFlush};
    }

    // Straight flush contains cards of the same suit. First check if
    // any suit appears more than 4 times. Later hand is filtered for ease of
    // use. Since the hand was sorted finding longest consecutive subsequence is
    // trivial.
    return_type CheckStraightFlush(hand_type hand) const {
      // Find potential flush suit. If one does not exist, return false and
      // uninitialized card.
      const std::optional<model::Card::Suit> potential_flush_suit =
          suit_count_mapping_.return_enum_for([](u8 value) {
            return value > 4;
          });

      if (!potential_flush_suit.has_value()) {
        return {false, {}};
      }

      // If potential flush exist find the longest consecutive sequence of
      // Cards.
      auto filtered =
          hand | std::ranges::views::filter([&](const model::Card& card) {
            return card.suit() == potential_flush_suit;
          });

      auto iterator = filtered.cbegin();
      model::Card::Rank previous_rank = iterator->rank();
      iterator++;

      u8 max_consecutive_counter = 0;
      // When the consecutive streak is greater or equal than 4 assign new value
      // to `streak_ending_card`. Counter starts from 0, so by the time it's at
      // 4, 5 consecutive numbers must have been seen.
      u8 consecutive_counter = 0;
      model::Card streak_ending_card;

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
          streak_ending_card =
              model::Card{potential_flush_suit.value(), iterator->rank()};
        }
        previous_rank = iterator->rank();
      }

      const bool result = max_consecutive_counter >= 4;

      return {result, CombinationType::kStraightFlush};
    }

    // Four of a kind is pretty self explanatorily.
    return_type CheckFourOfAKind([[maybe_unused]] hand_type hand) const {
      return {rank_count_mapping_
                  .return_enum_for([](u8 value) {
                    return value == 4;
                  })
                  .has_value(),
              CombinationType::kFourOfAKind};
    }

    return_type CheckFullHouse(hand_type hand) const {
      return_type pair_result = CheckPair(hand);
      return_type triplet_result = CheckThreeOfAKind(hand);

      return {pair_result.first && triplet_result.first,
              CombinationType::kFullHouse};
    }

    return_type CheckFlush(hand_type hand) const {
      return {suit_count_mapping_
                  .return_enum_for([](u8 value) {
                    return value == 5;
                  })
                  .has_value(),
              CombinationType::kFlush};
    }

    // Straight is defined as 5 cards with the consecutive ranks. Since the hand
    // is sorted
    return_type CheckStraight(hand_type hand) const {
      u8 consecutive_counter = 0;
      model::Card::Rank previous_rank = hand[0].rank();

      u8 max_consecutive_counter = 0;
      model::Card highest_card{};

      for (std::size_t i{1}; i < hand.size(); i++) {
        if (static_cast<i8>(hand[i].rank()) ==
            static_cast<i8>(previous_rank) + 1) {
          consecutive_counter++;
          max_consecutive_counter =
              std::max(max_consecutive_counter, consecutive_counter);
        } else {
          consecutive_counter = 0;
        }

        previous_rank = hand[i].rank();
        if (max_consecutive_counter >= 4) {
          highest_card = hand[i];
        }
      }

      return {max_consecutive_counter >= 4, CombinationType::kStraight};
    }

    return_type CheckThreeOfAKind([[maybe_unused]] hand_type hand) const {
      return {rank_count_mapping_
                  .return_enum_for([](u8 value) {
                    return value == 3;
                  })
                  .has_value(),
              CombinationType::kFourOfAKind};
    }

    return_type CheckTwoPair([[maybe_unused]] hand_type hand) const {
      // Check if there is at least one pair.
      const std::optional<model::Card::Rank> potential_rank_1 =
          rank_count_mapping_.return_enum_for([](u8 value) {
            return value == 2;
          });

      // Return if there isn't at least one pair.
      if (!potential_rank_1.has_value()) {
        return {false, {}};
      }

      // Try to find another pair.
      const u8 first_pair_rank = static_cast<u8>(potential_rank_1.value());
      return {rank_count_mapping_
                  .return_enum_for([first_pair_rank](u8 value) {
                    // Rank of the fairs pair should be omitted.
                    if (value == first_pair_rank) {
                      return false;
                    }
                    return value == 2;
                  })
                  .has_value(),
              CombinationType::kTwoPair};
    }

    return_type CheckPair([[maybe_unused]] hand_type hand) const {
      return {rank_count_mapping_
                  .return_enum_for([](u8 value) {
                    return value == 2;
                  })
                  .has_value(),
              CombinationType::kPair};
    }

    std::unordered_set<model::Card, decltype([](const model::Card& card) {
                         return static_cast<std::size_t>(card.value());
                       })>
        card_count_mapping_;

    utility::enum_indexable_array<model::Card::Suit, u8, 4> suit_count_mapping_;
    utility::enum_indexable_array<model::Card::Rank, u8, 13>
        rank_count_mapping_;
};

int main() {
  model::Deck deck;

  model::Card card1{model::Card::Suit::kHearts, model::Card::Rank::kKing};
  model::Card card2{model::Card::Suit::kDiamonds, model::Card::Rank::kKing};
  model::Card card3{model::Card::Suit::kHearts, model::Card::Rank::kTen};
  model::Card card4{model::Card::Suit::kDiamonds, model::Card::Rank::kTen};
  model::Card card5{model::Card::Suit::kHearts, model::Card::Rank::kFive};

  sorted_vector<model::Card> hand;
  hand.insert(card1);
  hand.insert(card2);
  hand.insert(card3);
  hand.insert(card4);
  hand.insert(card5);

  HandEvaluator evaluator;
  CombinationType combination = evaluator.Evaluate(hand.underlying());

  std::print("combination type {}\n", static_cast<u8>(combination));

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
