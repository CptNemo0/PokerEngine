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
// lobby and can be selected for the next match. If a participant disconnects
// midway through the game they will not be returned to the lobby. When a player
// disconnects they will be removed automatically from the connections_
// collection of the Server. This will make the reference stored in the players
// connection of MatchConductor the last one, thus when the game concludes the
// disconnected player will be destroyed.
class MatchConductor {
  public:
    enum class FinishReason {
      kNormal = 0,
      kServerFinished = 1,
      kPlayerLeft = 2,
      kReasonsNum
    };

    // MatchConductorManaged should move in the vector of Connections into
    // MachConductor's making MatchConductor a second owner of those players.
    // First one being the Server that has a "master" reference.
    MatchConductor(std::vector<std::shared_ptr<Server::Connection>> players,
                   Lobby& lobby,
                   MatchConductorManager& match_conductor_manager);
    ~MatchConductor();

    void ConductGame();

    bool HasFinished();
    void ForceFinish();

  private:
    // Returns the participants to the lobby. If a participant has disconnected
    // during the game or has left before the game had a chance to begin they
    // are not returned to the lobby and destroyed.
    void Finish();

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
