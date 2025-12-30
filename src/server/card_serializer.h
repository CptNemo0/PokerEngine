#ifndef SERVER_CARD_SERIALIZER_H_
#define SERVER_CARD_SERIALIZER_H_

#include <optional>
#include <string>
#include <string_view>

#include "card.h"

namespace utility {

class CardSerializer {
  public:
    static std::string Serialize(const model::Card& card);
    static std::optional<model::Card> Deserialize(std::string_view data);
};

} // namespace utility

#endif
