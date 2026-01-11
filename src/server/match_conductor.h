#ifndef SERVER_MATCH_CONDUCTOR_H_
#define SERVER_MATCH_CONDUCTOR_H_

#include <atomic>
#include <memory>
#include <vector>

#include "server.h"

namespace server {

class Lobby;
class MatchConductorManager;

// MatchConductor is responsible for conducting a singular game of poker. It is
// initialized with a collection of Connections allowing for communication with
// the participating players. When the game is concluded, players return to the
// lobby and can be selected for the next match.
class MatchConductor {
  public:
    enum class FinishReason {
      kNormal = 0,
      kServerFinished = 1,
      kPlayerLeft = 2,
      kReasonsNum
    };

    // MatchMaker will move in the array of Connections into MachConductor's
    // constructor for this game since it has not need for it.
    MatchConductor(std::vector<std::shared_ptr<Server::Connection>> players,
                   Lobby& lobby,
                   MatchConductorManager& match_conductor_manager);
    ~MatchConductor();

    // Conducts a game. If a player disconnects from the game the game is
    // terminated and all of the connected participants are returned to the
    // lobby.
    // I know that this is a rather extreme solution, but for now it will do the
    // trick.
    void ConductGame();

    bool HasFinished();

    void ForceFinish();

  private:
    // Returns the participants to the lobby. If a participant has disconnected
    // during the game or has left before the game had a chance to begin they
    // are not returned to the lobby.
    void FinishTheGame();

    // Participants.
    std::vector<std::shared_ptr<Server::Connection>> players_;
    // Reference to the lobby where players should return after the finished
    // game.
    Lobby& lobby_;

    std::atomic_bool stop_{false};
    std::atomic<FinishReason> finish_reason_;
};

} // namespace server

#endif // !SERVER_MATCH_CONDUCTOR_H_
