#include "match_maker.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <print>
#include <thread>
#include <utility>
#include <vector>
#include <winscard.h>

#include "connection_closure_handler.h"

#include "lobby.h"
#include "server_constants.h"
#include "server_manager.h"
#include "stacktrace_analyzer.h"

namespace server {

MatchMaker::MatchMaker(Lobby& lobby, ConnectionClosureHandler& closure_handler,
                       MatchConductorManager& match_conductor_manager)
  : lobby_(lobby), sever_manager_observation_(this),
    closure_handler_observation_(this),
    conductor_manager_(match_conductor_manager) {
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
  std::print("Matchmaker...");
  stop_.store(true);
  if (matchmaker_thread.joinable()) {
    matchmaker_thread.join();
  }
  stop_.store(false);
  std::print("finished\n");
}

size_t MatchMaker::OnConnectionClosed(u64 id) {
  auto remove_lambda =
    [id](const std::shared_ptr<Server::Connection>& connection) {
      if (!connection->closed.load()) {
        return false;
      }

      if (connection->id == id) {
        return true;
      } else {
        std::print("Connection was closed but the ids differ. Cooked!!!");
        common::utility::StacktraceAnalyzer::PrintOut();
        std::abort();
      }
    };

  std::lock_guard lock{buffer_mutex_};
  if (!intermediate_buffer_.size()) {
    return 0;
  }

  const size_t result = std::erase_if(intermediate_buffer_, remove_lambda);
  if (result) {
    std::print(
      "Connection {} erased from intermediate_buffer_. Buffer size: {}\n", id,
      intermediate_buffer_.size());
  }
  return result;
}

void MatchMaker::Run() {
  while (true) {
    if (stop_) {
      break;
    }
    if (lobby_.Empty()) {
      std::print("Lobby empty waiting\n");
      std::unique_lock lock{lobby_.mutex_};
      lobby_.data_cv_.wait_for(lock, 1s);
    }

    {
      std::scoped_lock lock{lobby_.mutex_, buffer_mutex_};
      if (lobby_.data_.empty() ||
          intermediate_buffer_.size() == gNumberOfPlayersInGame) {
      } else {
        std::print("Push back\n");
        intermediate_buffer_.push_back(std::move(lobby_.data_.front()));
        lobby_.data_.pop_front();
      }
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
  std::vector<std::shared_ptr<Server::Connection>> players(
    gNumberOfPlayersInGame);
  std::move(intermediate_buffer_.begin(), intermediate_buffer_.end(),
            players.begin());
  intermediate_buffer_.clear();
  lock.unlock();

  conductor_manager_.CreateMatchConductor(std::move(players), lobby_,
                                          conductor_manager_);
}

} // namespace server
