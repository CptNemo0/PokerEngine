#include "match_maker.h"

#include <algorithm>
#include <atomic>
#include <charconv>
#include <cstddef>
#include <memory>
#include <mutex>
#include <print>
#include <thread>
#include <utility>
#include <vector>
#include <winscard.h>

#include "connection_closure_handler.h"
#include "ixwebsocket/IXConnectionState.h"

#include "lobby.h"
#include "match_conductor.h"
#include "server_constants.h"
#include "server_manager.h"

namespace server {

MatchMaker::MatchMaker(Lobby& lobby, ConnectionClosureHandler& closure_handler)
  : lobby_(lobby), sever_manager_observation_(this),
    closure_handler_observation_(this) {
  sever_manager_observation_.Observe(std::addressof(ServerManager::Instance()));
  closure_handler_observation_.Observe(std::addressof(closure_handler));
  intermediate_buffer_.reserve(gNumberOfPlayersInGame);
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

size_t MatchMaker::OnConnectionClosed(ix::ConnectionState* state) {
  u64 id;
  std::from_chars(state->getId().data(),
                  state->getId().data() + state->getId().size(), id, 10);
  auto remove_lambda = [id](const Server::Connection& connection) {
    return connection.id == id;
  };

  std::lock_guard lock{buffer_mutex_};
  return std::erase_if(intermediate_buffer_, remove_lambda);
}

void MatchMaker::Run() {
  while (true) {
    if (stop_) {
      break;
    }
    std::print("Run\n");
    if (lobby_.Empty()) {
      std::print("Lobby empty waiting\n");
      std::unique_lock lock{lobby_.mutex_};
      lobby_.data_cv_.wait(lock);
    }

    {
      std::scoped_lock lock{lobby_.mutex_, buffer_mutex_};
      if (lobby_.data_.empty() ||
          intermediate_buffer_.size() == gNumberOfPlayersInGame) {
        std::print("Deque empty or Buffer full\n");
        return;
      }
      std::print("Push back\n");
      intermediate_buffer_.push_back(std::move(lobby_.data_.front()));
      lobby_.data_.pop_front();
    }

    {
      std::unique_lock lock{buffer_mutex_};
      if (intermediate_buffer_.size() == gNumberOfPlayersInGame) {
        std::print("Assemble game\n");
        AssembleGame(std::move(lock));
      }
    }
  }
}

void MatchMaker::AssembleGame(std::unique_lock<std::mutex> lock) {
  std::vector<Server::Connection> players(gNumberOfPlayersInGame);
  std::move(intermediate_buffer_.begin(), intermediate_buffer_.end(),
            players.begin());
  intermediate_buffer_.clear();
  lock.unlock();
  MatchConductor match_conductor{std::move(players), lobby_};
  std::jthread match_thread{&MatchConductor::ConductGame,
                            std::move(match_conductor)};
  match_thread.detach();
}

} // namespace server
