#include "game_assembler.h"

#include <memory>
#include <mutex>
#include <print>
#include <utility>

#include "ixwebsocket/IXWebSocket.h"

#include "lobby.h"
#include "server_constants.h"

namespace server {

GameAssembler::GameAssembler(Lobby& lobby) : lobby_(lobby) {
}

void GameAssembler::Run() {
  while (true) {
    {
      std::unique_lock lock{lobby_.mutex_};
      lobby_.data_cv_.wait(lock, [this] {
        return lobby_.data_.size() >= gNumberOfPlayersInGame;
      });

      for (auto i{0uz}; i < gNumberOfPlayersInGame; i++) {
        intermediate_buffer.push_back(std::move(lobby_.data_.front()));
        lobby_.data_.pop();
      }
    }

    AssembleGame();
  }
}

void GameAssembler::AssembleGame() {
  std::print("Creating a game\n");
  for (auto& connection : intermediate_buffer) {
    std::shared_ptr<ix::WebSocket> socket = connection.web_socket.lock();
    if (socket) {
      socket->send("Joining the game");
    }
  }
}

} // namespace server
