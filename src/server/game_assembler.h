#ifndef SERVER_GAME_ASSEMBLER_H_
#define SERVER_GAME_ASSEMBLER_H_

#include <vector>

#include "lobby.h"
#include "server.h"

namespace server {

class GameAssembler {
  public:
    explicit GameAssembler(Lobby& lobby);

    void Run();
    void AssembleGame();

  private:
    server::Lobby& lobby_;
    std::vector<server::Server::Connection> intermediate_buffer;
};

} // namespace server

#endif // !SERVER_GAME_ASSEMBLER_H_
