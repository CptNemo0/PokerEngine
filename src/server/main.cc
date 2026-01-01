#include <algorithm>
#include <array>
#include <cstddef>
#include <optional>
#include <print>
#include <ranges>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "aliasing.h"
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
      suit_count_mapping_.clear();
      rank_count_mapping_.clear();

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

  public:
    using return_type = std::pair<bool, CombinationType>;
    using hand_type = std::span<const model::Card>;
    using checker_function = return_type (HandEvaluator::*)(hand_type) const;

    // Royal flush contains cards 10 J Q K A of the same suit. First check
    // if any suit appears more than 4 times, than if those cards are
    // correct.
    return_type CheckRoyalFlush([[maybe_unused]] hand_type hand) const {
      const model::Card::Suit potential_flush_suit = [&]() {
        for (const auto& [key, value] : suit_count_mapping_) {
          if (value > 4) {
            return key;
          }
        }
        return model::Card::Suit::kNone;
      }();

      if (potential_flush_suit == model::Card::Suit::kNone) {
        return {false, CombinationType::kRoyalFlush};
      }

      const bool result =
          card_count_mapping_.contains(
              {potential_flush_suit, model::Card::Rank::kTen}) &&
          card_count_mapping_.contains(
              {potential_flush_suit, model::Card::Rank::kJack}) &&
          card_count_mapping_.contains(
              {potential_flush_suit, model::Card::Rank::kQueen}) &&
          card_count_mapping_.contains(
              {potential_flush_suit, model::Card::Rank::kKing}) &&
          card_count_mapping_.contains(
              {potential_flush_suit, model::Card::Rank::kAce});

      return {result, CombinationType::kRoyalFlush};
    }

    // Straight flush contains cards 10 J Q K A of the same suit. First check if
    // any suit appears more than 4 times. Later hand is filtered for ease of
    // use. Since the hand was sorted finding longest consecutive subsequence is
    // trivial.
    return_type CheckStraightFlush(hand_type hand) const {
      // Find potential flush suit. If one does not exist, return false and
      // uninitialized card.
      const model::Card::Suit potential_flush_suit = [&]() {
        for (const auto& [key, value] : suit_count_mapping_) {
          if (value > 4) {
            return key;
          }
        }
        return model::Card::Suit::kNone;
      }();

      if (potential_flush_suit == model::Card::Suit::kNone) {
        return {false, CombinationType::kStraightFlush};
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
              model::Card{potential_flush_suit, iterator->rank()};
        }
        previous_rank = iterator->rank();
      }

      const bool result = max_consecutive_counter >= 4;

      return {result, CombinationType::kStraightFlush};
    }

    // Four of a kind is pretty self explanatorily.
    return_type CheckFourOfAKind([[maybe_unused]] hand_type hand) const {
      for (const auto& [key, value] : rank_count_mapping_) {
        if (value == 4) {
          return {true, CombinationType::kFourOfAKind};
        }
      }
      return {false, CombinationType::kFourOfAKind};
    }

    return_type CheckFullHouse(hand_type hand) const {
      return_type pair_result = CheckPair(hand);
      return_type triplet_result = CheckThreeOfAKind(hand);

      return {pair_result.first && triplet_result.first,
              CombinationType::kFullHouse};
    }

    return_type CheckFlush(hand_type hand) const {
      for (const auto& [suit, count] : suit_count_mapping_) {
        if (count == 5) {
          // Since the hand is sorted in order of ascending rank, the first card
          // with the correct rank, when iterating from end to beginning, will
          // be the card with the greatest rank in the flush.
          for (std::size_t i{hand.size() - 1}; i > -1; i--) {
            if (hand[i].suit() == suit) {
              return {true, CombinationType::kFlush};
            }
          }
        }
      }
      return {false, CombinationType::kFlush};
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
      u8 triplet_counter = 0;
      for (const auto& [rank, count] : rank_count_mapping_) {
        if (count == 3) {
          triplet_counter++;
        }
      }
      return {triplet_counter >= 1, CombinationType::kThreeOfAKind};
    }

    return_type CheckTwoPair([[maybe_unused]] hand_type hand) const {
      u8 pair_counter = 0;
      for (const auto& [rank, count] : rank_count_mapping_) {
        if (count == 2) {
          pair_counter++;
        }
      }
      return {pair_counter == 2, CombinationType::kTwoPair};
    }

    return_type CheckPair([[maybe_unused]] hand_type hand) const {
      u8 pair_counter = 0;
      for (const auto& [rank, count] : rank_count_mapping_) {
        if (count == 2) {
          pair_counter++;
        }
      }
      return {pair_counter == 1, CombinationType::kPair};
    }

    std::unordered_set<model::Card, decltype([](const model::Card& card) {
                         return static_cast<std::size_t>(card.value());
                       })>
        card_count_mapping_;
    std::unordered_map<model::Card::Suit, u8> suit_count_mapping_;
    std::unordered_map<model::Card::Rank, u8> rank_count_mapping_;
};

int main() {
  model::Deck deck;

  model::Card card1{model::Card::Suit::kDiamonds, model::Card::Rank::kKing};
  model::Card card2{model::Card::Suit::kHearts, model::Card::Rank::kKing};
  model::Card card3{model::Card::Suit::kClubs, model::Card::Rank::kQueen};
  model::Card card4{model::Card::Suit::kHearts, model::Card::Rank::kQueen};
  model::Card card5{model::Card::Suit::kSpades, model::Card::Rank::kQueen};

  sorted_vector<model::Card> hand;
  hand.insert(card1);
  hand.insert(card2);
  hand.insert(card3);
  hand.insert(card4);
  hand.insert(card5);

  HandEvaluator evaluator;
  CombinationType combination = evaluator.Evaluate(hand.underlying());

  std::print("combination type {}\n", static_cast<i8>(combination));

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
