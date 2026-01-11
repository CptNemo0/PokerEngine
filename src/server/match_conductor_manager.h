#ifndef SERVER_MATCH_CONDUCTOR_MANAGER_H_
#define SERVER_MATCH_CONDUCTOR_MANAGER_H_

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

#include "aliasing.h"
#include "match_conductor.h"
#include "scoped_observation.h"
#include "server.h"
#include "server_manager.h"

namespace server {

class MatchConductor;

class MatchConductorManager : public ServerManager::Observer {
  public:
    MatchConductorManager();
    void CreateMatchConductor(
      std::vector<std::shared_ptr<Server::Connection>> connections,
      Lobby& lobby, MatchConductorManager& conductor_manager);
    void MapIdToConductor(u64 id, MatchConductor* conductor);
    void RemoveIdConductorMapping(u64 id, MatchConductor* conductor);

    virtual void Start() override {};

    virtual void End() override;

  private:
    std::atomic_bool finish_requested{false};
    std::mutex conductors_mutex_;
    std::vector<std::pair<std::unique_ptr<MatchConductor>, std::jthread>>
      match_conductors_;
    common::utility::ScopedObservation<ServerManager, MatchConductorManager>
      server_manager_observation_;
};

} // namespace server

#endif // !SERVER_MATCH_CONDUCTOR_MANAGER_H_
