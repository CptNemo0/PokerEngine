#ifndef SERVER_SERVER_H_
#define SERVER_SERVER_H_

#include <atomic>
#include <cstdint>
#include <limits>
#include <memory>
#include <mutex>
#include <print>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "connection_closure_handler.h"
#include "ixwebsocket/IXConnectionState.h"
#include "ixwebsocket/IXWebSocket.h"
#include "ixwebsocket/IXWebSocketMessage.h"
#include "ixwebsocket/IXWebSocketServer.h"

#include "aliasing.h"
#include "scoped_observation.h"
#include "server_manager.h"

namespace server {

class Lobby;

class Server : public ServerManager::Observer {
  public:
    struct Connection {
        std::weak_ptr<ix::WebSocket> web_socket{};
        std::shared_ptr<ix::ConnectionState> state{};
        u64 id{(std::numeric_limits<u64>::max)()};
        std::atomic_bool closed{false};

        Connection(std::weak_ptr<ix::WebSocket> socket,
                   std::shared_ptr<ix::ConnectionState> connection_state)
          : web_socket(socket), state(connection_state),
            id(std::stoull(connection_state->getId())) {
          std::print("Connection {} constructed\n", id);
        }
        Connection(const Connection& other) noexcept
          : Connection(other.web_socket, other.state) {
          std::print("Connection {} copied\n", id);
        }
        Connection(Connection&& other) noexcept
          : web_socket(std::move(other.web_socket)),
            state(std::move(other.state)), id(other.id) {
          std::print("Connection {} moved\n", id);
        };

        void operator=(const Connection& other) noexcept {
          web_socket = other.web_socket;
          state = other.state;
          id = other.id;
          std::print("Connection {} copy assigned\n", id);
        }
        void operator=(Connection&& other) noexcept {
          web_socket = std::move(other.web_socket);
          state = std::move(other.state);
          id = std::exchange(other.id,
                             (std::numeric_limits<std::uint64_t>::max)());
          std::print("Connection {} move assigned\n", id);
        }

        ~Connection() {
          std::print("Connection destroyed\n");
        }
    };

    Server(int port, const std::string_view& host, Lobby& lobby,
           ConnectionClosureHandler& closure_handler);

    virtual void Start() override;

    virtual void End() override;

    void OnNewConnectionEstablished(std::weak_ptr<ix::WebSocket> web_socket,
                                    std::shared_ptr<ix::ConnectionState> state);

    void OnConnectionClosed(const ix::ConnectionState& state);

    void AddObserver(Observer* observer);

    void RemoveObserver(Observer* observer);

    void OnConnectionClosed(ix::ConnectionState* state);

  private:
    struct MessageHandler {
        std::weak_ptr<ix::WebSocket> web_socket{};
        std::shared_ptr<ix::ConnectionState> state{};
        Server* server{nullptr};

        void operator()(const ix::WebSocketMessagePtr& msg);
    };

    std::mutex connections_mutex_;
    std::vector<std::shared_ptr<Connection>> connections_;

    std::atomic_bool stop_{false};

    std::unique_ptr<ix::WebSocketServer> server_;
    Lobby& lobby_;
    ConnectionClosureHandler& closure_handler_;

    common::utility::ScopedObservation<ServerManager, Server>
      server_manager_observation_;
};

} // namespace server

#endif // !SERVER_SERVER_H_
