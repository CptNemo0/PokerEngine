#include "match_maker.h"

#include <atomic>
#include <memory>
#include <thread>
#include <utility>

#include "lobby.h"
#include "match_conductor.h"
#include "server_constants.h"

namespace {

std::atomic_bool should_end{false};

} // namespace

namespace server {

MatchMaker::MatchMaker(Lobby& lobby) : lobby_(lobby) {
}

void MatchMaker::Start() {
  u64 current_index = 0;
  while (true) {
    if (lobby_.WaitPop(intermediate_buffer_[current_index])) {
      current_index++;
    }

    if (should_end.load(std::memory_order::acquire)) {
      break;
    }

    if (current_index == 3) {
      AssembleGame();
      current_index = 0;
    }
  }

  for (auto i{0uz}; i < current_index; i++) {
    lobby_.Push(std::move(intermediate_buffer_[i]));
  }
}

void MatchMaker::Stop() {
  should_end.store(true, std::memory_order_release);
}

void MatchMaker::AssembleGame() {
  std::unique_ptr<MatchConductor> match_conductor =
    std::make_unique<MatchConductor>(std::move(intermediate_buffer_), lobby_);

  std::jthread game_thread{&MatchConductor::ConductGame,
                           std::move(match_conductor)};
  game_thread.detach();
}

} // namespace server
