#ifndef COMMON_CARD_SERIALIZER_H_
#define COMMON_CARD_SERIALIZER_H_

#include <optional>
#include <string>
#include <string_view>

#include "model/card.h"

namespace utility {

// `CardSerializer` is a utility class that serializes and deserializes `Card`
// objects.
class CardSerializer {
  public:
    // Serializes a card to a string.
    static std::string Serialize(const model::Card& card);

    // Deserializes a card from a string. If string is malformed or has
    // incorrect data an std::nullopt is returned.
    static std::optional<model::Card> Deserialize(std::string_view data);
};

} // namespace utility

#endif // !COMMON_CARD_SERIALIZER_H_
