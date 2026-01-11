#include "server_manager.h"

#include <memory>
#include <mutex>
#include <print>
#include <ranges>
#include <thread>

#include "connection_closure_handler.h"
#include "lobby.h"
#include "match_conductor_manager.h"
#include "match_maker.h"
#include "server.h"
#include "server_constants.h"

namespace server {

ServerManager::ServerManager() {
}

ServerManager::~ServerManager() {
}

void ServerManager::AddObserver(Observer* observer) {
  std::lock_guard lock{observers_mutex_};
  observers_.push_back(observer);
}

void ServerManager::RemoveObserver(Observer* observer) {
  std::lock_guard lock{observers_mutex_};
  auto _ = std::erase(observers_, observer);
}

void ServerManager::Initialize() {
  connection_closure_handler_ = std::make_unique<ConnectionClosureHandler>();
  lobby_ = std::make_unique<Lobby>(*(connection_closure_handler_.get()));
  match_conductor_manager_ = std::make_unique<MatchConductorManager>();
  server_ =
    std::make_unique<Server>(server::gPort, server::gHost, *lobby_.get(),
                             *connection_closure_handler_.get());
  match_maker_ = std::make_unique<MatchMaker>(
    *lobby_.get(), *connection_closure_handler_.get(),
    *match_conductor_manager_.get());
}

void ServerManager::Start() {
  std::lock_guard lock{observers_mutex_};
  for (Observer* observer : observers_) {
    observer->Start();
  }
}

void ServerManager::Wait() {
  // Start listener_thread
  std::jthread listener_thread{&Waiter::Listen, Waiter{}};
  std::unique_lock lock{wait_mutex_};
  // Block until Waiter notifies that user typed "q" or "quit"
  wait_cv_.wait(lock);
}

void ServerManager::End() {
  std::lock_guard lock{observers_mutex_};
  // I iterate in the reverse pattern since I want to explicitly ensure that
  // observers created later, that are relying on the ones created first are
  // destroyed earlier. I will probably think of a better solution since I see
  // how fragile this one is, but for now it's also a cool flex with
  // views::reverse.
  std::print("Ending\n");
  for (Observer* observer : observers_ | std::views::reverse) {
    observer->End();
  }
}

} // namespace server
