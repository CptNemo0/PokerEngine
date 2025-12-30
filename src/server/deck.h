#ifndef SERVER_DECK_H_
#define SERVER_DECK_H_

#include <array>
#include <cstddef>
#include <optional>
#include <random>

#include "card.h"

namespace model {

constexpr std::size_t gDeckSize = 52;

class Deck {
  public:
    Deck();

    // Shuffles the `card_` array and resets the `deal_pointer_`.
    // It wouldn't make sense to keep the old value of the `deal_pointer_`.
    void Reshuffle();

    // Returns a card or nullopt if the whole deck has been dealt already.
    std::optional<Card> Deal();

    bool AnyCardsLeft() {
      return deal_pointer_ < gDeckSize;
    }

  private:
    // The cards array. It stores all the cards used for the game.
    std::array<Card, gDeckSize> cards_;

    // This is a shuffled view of the `cards_` array. Instead of accessing
    // `cards_` directly there is a view created that orders cards randomly.
    // EXPLORE: Is there a way to make `cards_` array implicit?
    std::array<u32, gDeckSize> shuffled_view_;

    // The `deal_pointer_` stores an index of a card that will be delt next.
    u32 deal_pointer_{0};

    // Random number generator members.
    std::random_device random_device_;
    std::mt19937 generator_{random_device_()};
};

} // namespace model

#endif // !SERVER_DECK_H_
