#include "match_maker.h"

#include <atomic>
#include <memory>
#include <thread>
#include <utility>

#include "lobby.h"
#include "match_conductor.h"
#include "server_constants.h"
#include "server_manager.h"

namespace server {

MatchMaker::MatchMaker(Lobby& lobby)
  : lobby_(lobby), sever_manager_observation_(this) {
  sever_manager_observation_.Observe(std::addressof(ServerManager::Instance()));
}

void MatchMaker::Start() {
  if (!matchmaker_thread.joinable()) {
    matchmaker_thread = std::thread(&MatchMaker::Run, this);
  }
}

void MatchMaker::End() {
  stop_.store(true);
  if (matchmaker_thread.joinable()) {
    matchmaker_thread.join();
  }
  stop_.store(false);
}

void MatchMaker::Run() {
  u64 current_index = 0;
  while (true) {
    if (stop_) {
      break;
    }

    if (lobby_.WaitPop(intermediate_buffer_[current_index])) {
      current_index++;
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

void MatchMaker::AssembleGame() {
  std::unique_ptr<MatchConductor> match_conductor =
    std::make_unique<MatchConductor>(std::move(intermediate_buffer_), lobby_);

  std::jthread game_thread{&MatchConductor::ConductGame,
                           std::move(match_conductor)};
  game_thread.detach();
}

} // namespace server
