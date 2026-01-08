#ifndef SERVER_MATCH_MAKER_H_
#define SERVER_MATCH_MAKER_H_

#include <array>

#include "lobby.h"
#include "server.h"
#include "server_constants.h"

namespace server {

// MatchMaker class is responsible for assembling players for a game. When a new
// player enters a lobby, MatchMaker is awoken by the conditional variable. It
// moves the new player form the lobby to the intermediate_buffer_. When
// gNumberOfPlayersInGame has been moved a new MatchConductor object is created
// and contents of the intermediate_buffer is moved to it.
class MatchMaker {
  public:
    explicit MatchMaker(Lobby& lobby);

    // Starts a MatchMaker loop. It should be called as an argument for a
    // thread/jthread constructor.
    void Start();

    // Creates a MatchConductor, passes the ownership of the players currently
    // in the intermediate_buffer_ to it, and starts the game.
    void AssembleGame();

  private:
    server::Lobby& lobby_;
    std::array<Server::Connection, gNumberOfPlayersInGame> intermediate_buffer_;
};

} // namespace server

#endif // !SERVER_MATCHMAKER_H_
