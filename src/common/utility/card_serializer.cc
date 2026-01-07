#include "card_serializer.h"

#include <exception>
#include <format>
#include <optional>
#include <print>
#include <string>
#include <string_view>

#include "model/card.h"

namespace common::utility {

namespace {

model::Card::Suit CharToSuit(char character) {
  using Suit = model::Card::Suit;
  switch (character) {
  case 'S':
    return Suit::kSpades;
  case 'C':
    return Suit::kClubs;
  case 'D':
    return Suit::kDiamonds;
  case 'H':
    return Suit::kHearts;
  default:
    throw std::exception("Could not parse the character to suit");
  }
}

model::Card::Rank CharToRank(char character) {
  using Rank = model::Card::Rank;
  switch (character) {
  case '2':
    return Rank::kTwo;
  case '3':
    return Rank::kThree;
  case '4':
    return Rank::kFour;
  case '5':
    return Rank::kFive;
  case '6':
    return Rank::kSix;
  case '7':
    return Rank::kSeven;
  case '8':
    return Rank::kEighth;
  case '9':
    return Rank::kNine;
  case 'T':
    return Rank::kTen;
  case 'A':
    return Rank::kAce;
  case 'J':
    return Rank::kJack;
  case 'Q':
    return Rank::kQueen;
  case 'K':
    return Rank::kKing;
  default:
    throw std::exception{};
  }
}

char SuitToChar(model::Card::Suit suit) {
  using Suit = model::Card::Suit;
  switch (suit) {
  case Suit::kSpades:
    return 'S';
  case Suit::kClubs:
    return 'C';
  case Suit::kDiamonds:
    return 'D';
  case Suit::kHearts:
    return 'H';
  }
}

char RankToChar(model::Card::Rank rank) {
  using Rank = model::Card::Rank;
  switch (rank) {
  case Rank::kTwo:
    return '2';
  case Rank::kThree:
    return '3';
  case Rank::kFour:
    return '4';
  case Rank::kFive:
    return '5';
  case Rank::kSix:
    return '6';
  case Rank::kSeven:
    return '7';
  case Rank::kEighth:
    return '8';
  case Rank::kNine:
    return '9';
  case Rank::kTen:
    return 'T';
  case Rank::kAce:
    return 'A';
  case Rank::kJack:
    return 'J';
  case Rank::kQueen:
    return 'Q';
  case Rank::kKing:
    return 'K';
  }
}

} // namespace

std::string CardSerializer::Serialize(const model::Card& card) {
  return std::format("{}{}", SuitToChar(card.suit()), RankToChar(card.rank()));
}

std::optional<model::Card> CardSerializer::Deserialize(std::string_view data) {
  if (data.length() != 2) {
    return std::nullopt;
  }

  model::Card::Suit suit{};
  model::Card::Rank rank{};
  try {
    suit = CharToSuit(data.at(0));
  } catch (const std::exception& e) {
    std::print("Could not parse character \"{}\"to suit.\n", data.at(0));
    return std::nullopt;
  }

  try {
    rank = CharToRank(data.at(1));
  } catch (const std::exception& e) {
    std::print("Could not parse character \"{}\"to rank.\n", data.at(0));
    return std::nullopt;
  }
  return model::Card{suit, rank};
}

} // namespace common::utility
