#ifndef SERVER_MATCH_MAKER_H_
#define SERVER_MATCH_MAKER_H_

#include <atomic>
#include <cstddef>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "connection_closure_handler.h"
#include "lobby.h"
#include "match_conductor_manager.h"
#include "scoped_observation.h"
#include "server.h"
#include "server_manager.h"

namespace server {

// MatchMaker class is responsible for assembling players for a game. It
// cooperates very closely with the lobby class. It pops players waiting in the
// lobby and assembles games out of them.
// Implements both
// 1. ServerManager::Observer to know when to start and when to
// finish execution,
// 2. ConnectionClosureHandler::Observer to check whether the player that
// disconnected was one of the players in the intermediate buffer.
class MatchMaker : public ServerManager::Observer,
                   public ConnectionClosureHandler::Observer {
  public:
    MatchMaker(Lobby& lobby, ConnectionClosureHandler& closure_handler,
               MatchConductorManager& conductor_manager);

    // Creates a matchmaker_thread that will execute Run() method.
    virtual void Start() override;

    // Sets stop_ to false, than waits for the matchmaker_thread to join.
    virtual void End() override;

    virtual size_t OnConnectionClosed(u64 id) override;

  private:
    // Executes MatchMaker main loop. It should be called as an argument for a
    // thread/jthread constructor.
    void Run();

    // Creates a MatchConductor, passes the ownership of the players currently
    // in the intermediate_buffer_ to it, and starts the game.
    void AssembleGame(std::unique_lock<std::mutex> lock);

    std::mutex buffer_mutex_;

    std::thread matchmaker_thread;
    std::atomic<bool> stop_{false};

    Lobby& lobby_;
    MatchConductorManager& conductor_manager_;
    std::vector<std::shared_ptr<Server::Connection>> intermediate_buffer_;

    common::utility::ScopedObservation<ServerManager, MatchMaker>
      sever_manager_observation_;

    common::utility::ScopedObservation<ConnectionClosureHandler, MatchMaker>
      closure_handler_observation_;
};

} // namespace server

#endif // !SERVER_MATCHMAKER_H_
