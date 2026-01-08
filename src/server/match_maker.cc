#include "match_maker.h"

#include <memory>
#include <thread>
#include <utility>

#include "lobby.h"
#include "match_conductor.h"
#include "server_constants.h"
#include "stacktrace_analyzer.h"

namespace server {

MatchMaker::MatchMaker(Lobby& lobby) : lobby_(lobby) {
}

void MatchMaker::Start() {
  TRACE_CURRENT_FUNCTION();
  u64 current_index = 0;
  while (true) {
    lobby_.WaitPop(intermediate_buffer_[current_index++]);

    if (current_index == 3) {
      AssembleGame();
      current_index = 0;
    }
  }
}

void MatchMaker::AssembleGame() {
  TRACE_CURRENT_FUNCTION();
  std::unique_ptr<MatchConductor> match_conductor =
    std::make_unique<MatchConductor>(std::move(intermediate_buffer_), lobby_);

  std::jthread game_thread{&MatchConductor::ConductGame,
                           std::move(match_conductor)};
  game_thread.detach();
}

} // namespace server
