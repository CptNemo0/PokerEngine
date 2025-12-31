#include <algorithm>
#include <cstddef>
#include <optional>
#include <print>
#include <ranges>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>

#include "aliasing.h"
#include "model/card.h"
#include "model/deck.h"
#include "utility/card_serializer.h"
#include "utility/sorted_vector.h"

class Combination {
  public:
    enum class CombinationType : i8 {
      kRoyalFlush = 0,
      kStraightFlush = 1,
      kHighCard = 2
    };

    Combination(CombinationType type, const model::Card& differentiator)
        : type_(type), differentiator_(differentiator) {
    }

    // private:
    CombinationType type_;
    model::Card differentiator_;
};

class HandEvaluator {
  public:
    Combination Evaluate(std::span<const model::Card> hand) {
      card_count_mapping_.clear();
      suit_count_mapping_.clear();
      rank_count_mapping_.clear();

      for (const auto& card : hand) {
        card_count_mapping_.insert(card);
        suit_count_mapping_[card.suit()]++;
        rank_count_mapping_[card.rank()]++;
      }

      {
        const auto [result, suit] = CheckRoyalFlush();
        if (result) {
          return {Combination::CombinationType::kRoyalFlush,
                  model::Card{suit, model::Card::Rank::kAce}};
        }
      }

      {
        const auto [result, card] = CheckStraightFlush(hand);
        if (result) {
          return {Combination::CombinationType::kStraightFlush, card};
        }
      }

      return {Combination::CombinationType::kHighCard, hand.back()};
    }

    // Royal flush contains cards 10 J Q K A of the same suit. First check if
    // any suit appears more than 4 times, than if those cards are correct.
    std::pair<bool, const model::Card::Suit> CheckRoyalFlush() {
      const model::Card::Suit potentaial_flush_suit = [&]() {
        for (const auto& [key, value] : suit_count_mapping_) {
          if (value > 4) {
            return key;
          }
        }
        return model::Card::Suit::kNone;
      }();

      if (potentaial_flush_suit == model::Card::Suit::kNone) {
        return std::make_pair(false, model::Card::Suit::kNone);
      }

      const bool result =
          card_count_mapping_.contains(
              {potentaial_flush_suit, model::Card::Rank::kTen}) &&
          card_count_mapping_.contains(
              {potentaial_flush_suit, model::Card::Rank::kJack}) &&
          card_count_mapping_.contains(
              {potentaial_flush_suit, model::Card::Rank::kQueen}) &&
          card_count_mapping_.contains(
              {potentaial_flush_suit, model::Card::Rank::kKing}) &&
          card_count_mapping_.contains(
              {potentaial_flush_suit, model::Card::Rank::kAce});

      return std::make_pair(result, result ? potentaial_flush_suit
                                           : model::Card::Suit::kNone);
    }

    // Royal flush contains cards 10 J Q K A of the same suit. First check if
    // any suit appears more than 4 times. Later hand is filtered for ease of
    // use. Since the hand was sorted finding longest consecutive subsequence is
    // trivial.
    std::pair<bool, const model::Card>
    CheckStraightFlush(std::span<const model::Card> hand) {
      // Find potential flush suit. If one does not exist, return false and
      // uninitialized card.
      const model::Card::Suit potentaial_flush_suit = [&]() {
        for (const auto& [key, value] : suit_count_mapping_) {
          if (value > 4) {
            return key;
          }
        }
        return model::Card::Suit::kNone;
      }();

      if (potentaial_flush_suit == model::Card::Suit::kNone) {
        return std::make_pair(false, model::Card{});
      }

      // If potential flush exist find the longest consecutive sequence of
      // Cards.
      auto filtered =
          hand | std::ranges::views::filter([&](const model::Card& card) {
            return card.suit() == potentaial_flush_suit;
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
          if (consecutive_counter >= 4) {
            streak_ending_card =
                model::Card{potentaial_flush_suit, iterator->rank()};
          }
        } else {
          consecutive_counter = 0;
        }
        previous_rank = iterator->rank();
      }

      const bool result = max_consecutive_counter >= 4;

      return std::make_pair(result,
                            result ? streak_ending_card : model::Card{});
    }

    std::pair<bool, const model::Card> CheckFourOfAKind() {
      for (const auto& [key, value] : rank_count_mapping_) {
        if (value == 4) {
          return {true, model::Card{model::Card::Suit::kHearts, key}};
        }
      }
      return {false, model::Card{}};
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

  model::Card card1{model::Card::Suit::kHearts, model::Card::Rank::kTen};
  model::Card card2{model::Card::Suit::kHearts, model::Card::Rank::kJack};
  model::Card card3{model::Card::Suit::kHearts, model::Card::Rank::kQueen};
  model::Card card4{model::Card::Suit::kHearts, model::Card::Rank::kKing};
  model::Card card5{model::Card::Suit::kHearts, model::Card::Rank::kAce};

  sorted_vector<model::Card> hand;
  hand.insert(card1);
  hand.insert(card2);
  hand.insert(card3);
  hand.insert(card4);
  hand.insert(card5);

  HandEvaluator evaluator;
  Combination combination = evaluator.Evaluate(hand.underlying());

  std::print("combination type {} | diffrentiator: {}", (i8)combination.type_,
             (i8)std::get<model::Card::Suit>(combination.differentiator_));

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
