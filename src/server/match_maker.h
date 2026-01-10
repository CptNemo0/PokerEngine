#ifndef SERVER_MATCH_MAKER_H_
#define SERVER_MATCH_MAKER_H_

#include <array>
#include <atomic>
#include <thread>

#include "lobby.h"
#include "scoped_observation.h"
#include "server.h"
#include "server_constants.h"
#include "server_manager.h"

namespace server {

// MatchMaker class is responsible for assembling players for a game. When a new
// player enters a lobby, MatchMaker is awoken by the conditional variable. It
// moves the new player form the lobby to the intermediate_buffer_. When
// gNumberOfPlayersInGame has been moved a new MatchConductor object is created
// and contents of the intermediate_buffer is moved to it. When Matchmaker is
// ordered to finish, it will return it's players to the Lobby.
class MatchMaker : public ServerManager::Observer {
  public:
    explicit MatchMaker(Lobby& lobby);

    // Creates a matchmaker_thread that will execute Run() method.
    virtual void Start() override;

    // Sets stop_ to false, than waits for the matchmaker_thread to join.
    virtual void End() override;

  private:
    // Executes MatchMaker main loop. It should be called as an argument for a
    // thread/jthread constructor.
    void Run();

    // Creates a MatchConductor, passes the ownership of the players currently
    // in the intermediate_buffer_ to it, and starts the game.
    void AssembleGame();

    std::thread matchmaker_thread;
    std::atomic<bool> stop_{false};

    server::Lobby& lobby_;
    std::array<Server::Connection, gNumberOfPlayersInGame> intermediate_buffer_;

    common::utility::ScopedObservation<ServerManager, MatchMaker>
      sever_manager_observation_;
};

} // namespace server

#endif // !SERVER_MATCHMAKER_H_
