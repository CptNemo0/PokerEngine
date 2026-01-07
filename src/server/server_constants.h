#ifndef SERVER_CONSTANTS_H_
#define SERVER_CONSTANTS_H_

#include "aliasing.h"
#include <string_view>

namespace server {

static inline constexpr std::string_view gHost = "127.0.0.1";

static inline constexpr i32 gPort = 8008;

static inline constexpr u64 gNumberOfPlayersInGame = 3;

} // namespace server

#endif // !SERVER_CONSTANTS_H_
