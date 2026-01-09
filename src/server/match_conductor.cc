#include <array>
#include <chrono>
#include <format>
#include <print>
#include <thread>
#include <utility>

#include "lobby.h"
#include "match_conductor.h"
#include "server.h"
#include "server_constants.h"

namespace server {

using namespace std::literals::chrono_literals;

MatchConductor::MatchConductor(
  std::array<Server::Connection, gNumberOfPlayersInGame>&& players,
  Lobby& lobby)
  : players_(std::move(players)), lobby_(lobby) {
  // Placeholder logic
  std::print("Starting a game with players: ");
  for (const auto& player : players) {
    std::print("{} ", player.id);
  }
  std::print("\n");
};

// Placeholder logic
MatchConductor::~MatchConductor() {
  std::print("MatchConductor Destructor\n");
}

// Placeholder logic
void MatchConductor::ConductGame() {
  for (const auto& player : players_) {
    if (auto ws = player.web_socket.lock()) {
      ws->send(std::format("Welcome to the game player: {}", player.id));
    }
  }

  std::this_thread::sleep_for(10s);

  for (const auto& player : players_) {
    if (auto ws = player.web_socket.lock()) {
      ws->send("Game finished\n");
    }
  }

  ReturnPlayersToTheLobby();
}

void MatchConductor::ReturnPlayersToTheLobby() {
  for (auto& player : players_) {
    if (!player.state->isTerminated()) {
      lobby_.Push(std::move(player));
    }
  }
}

} // namespace server
