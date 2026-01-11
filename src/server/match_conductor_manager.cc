#include "match_conductor_manager.h"

#include <cstddef>
#include <memory>
#include <mutex>
#include <print>
#include <thread>
#include <utility>
#include <vector>

#include "lobby.h"
#include "match_conductor.h"
#include "server.h"
#include "server_manager.h"

namespace server {

MatchConductorManager::MatchConductorManager()
  : server_manager_observation_(this) {
  server_manager_observation_.Observe(
    std::addressof(ServerManager::Instance()));
}

void MatchConductorManager::CreateMatchConductor(
  std::vector<std::shared_ptr<Server::Connection>> connections, Lobby& lobby,
  MatchConductorManager& conductor_manager) {
  if (finish_requested) {
    return;
  }

  {
    // Cleans up finished games. J threads will join automatically upon removal.
    std::lock_guard lock{conductors_mutex_};

    std::size_t result = std::erase_if(
      match_conductors_,
      [](const std::pair<std::unique_ptr<MatchConductor>, std::jthread>& p) {
        return p.first->HasFinished() && p.second.joinable();
      });
  }

  // Creates a new MatchConductor and starts its thread.
  std::unique_ptr<MatchConductor> match_conductor =
    std::make_unique<MatchConductor>(std::move(connections), lobby,
                                     conductor_manager);

  std::jthread match_thread{&MatchConductor::ConductGame,
                            match_conductor.get()};

  {
    std::lock_guard lock{conductors_mutex_};
    match_conductors_.push_back(
      {std::move(match_conductor), std::move(match_thread)});
  }
}

void MatchConductorManager::End() {
  std::print("MatchConductorManager ... ");
  finish_requested = true;
  std::lock_guard lock{conductors_mutex_};
  for (const auto& [pointer, thread] : match_conductors_) {
    pointer->ForceFinish();
  }
  std::print("finished.\n");
}

} // namespace server
