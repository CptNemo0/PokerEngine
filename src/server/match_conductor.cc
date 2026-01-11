#include "match_conductor.h"

#include <chrono>
#include <format>
#include <memory>
#include <print>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "lobby.h"
#include "match_conductor_manager.h"
#include "server.h"

namespace {

std::string_view
FinishReasonToString(server::MatchConductor::FinishReason reason) {
  switch (reason) {
  case server::MatchConductor::FinishReason::kNormal:
    return "The game has finished normally";
  case server::MatchConductor::FinishReason::kServerFinished:
    return "The server has forced the game to finish";
  case server::MatchConductor::FinishReason::kPlayerLeft:
    return "A player has left";
  }
}

} // namespace

namespace server {

using namespace std::literals::chrono_literals;

MatchConductor::MatchConductor(
  std::vector<std::shared_ptr<Server::Connection>> players, Lobby& lobby,
  MatchConductorManager& match_conductor_manager)
  : players_(std::move(players)), lobby_(lobby) {

  for (auto& player : players_) {
    if (player->closed.load()) {
      std::print("Player {} has disconnected\n", player->id);
    }
  }

  std::print("Starting a game with players: ");

  for (auto& player : players_) {
    std::print("{} ", player->id);
  }

  std::print("\n");
};

// Placeholder logic
MatchConductor::~MatchConductor() {
  std::print("MatchConductor Destructor\n");
}

// Placeholder logic
void MatchConductor::ConductGame() {
  while (true) {
    if (stop_) {
      break;
    }

    for (const auto& player : players_) {
      if (auto ws = player->web_socket.lock()) {
        ws->send(std::format("Welcome to the game player: {}", player->id));
      } else {
        finish_reason_.store(FinishReason::kPlayerLeft);
        FinishTheGame();
      }
    }

    std::this_thread::sleep_for(10s);

    FinishTheGame();
  }
}

bool MatchConductor::HasFinished() {
  return stop_;
}

void MatchConductor::ForceFinish() {
  stop_ = true;
}

void MatchConductor::FinishTheGame() {
  for (auto& player : players_) {
    if (!player->state->isTerminated()) {
      if (auto ws = player->web_socket.lock()) {
        ws->send(FinishReasonToString(finish_reason_.load()).data());
      }
      if (!stop_) {
        lobby_.Push(std::move(player));
      }
    }
  }
  stop_ = true;
}

} // namespace server
