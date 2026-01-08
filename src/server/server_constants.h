#ifndef SERVER_CONSTANTS_H_
#define SERVER_CONSTANTS_H_

#include "aliasing.h"
#include <string_view>

namespace server {

inline constexpr std::string_view gHost = "127.0.0.1";

inline constexpr i32 gPort = 8008;

inline constexpr u64 gNumberOfPlayersInGame = 3;

inline constexpr u64 gMaxConcurrentConnections = 64;

static inline constexpr u64 gMaxConnectionsInTheLobby = 64;

} // namespace server

#endif // !SERVER_CONSTANTS_H_
