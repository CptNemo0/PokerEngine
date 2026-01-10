#ifndef SERVER_SERVER_H_
#define SERVER_SERVER_H_

#include <cstdint>
#include <limits>
#include <memory>
#include <mutex>
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
    class Observer {
      public:
        virtual void OnConnectionClosed(ix::ConnectionState* state) = 0;
    };

    struct Connection {
        std::weak_ptr<ix::WebSocket> web_socket{};
        std::shared_ptr<ix::ConnectionState> state{};
        std::uint64_t id{(std::numeric_limits<std::uint64_t>::max)()};

        Connection() = default;

        Connection(std::weak_ptr<ix::WebSocket> socket,
                   std::shared_ptr<ix::ConnectionState> connection_state)
          : web_socket(socket), state(connection_state),
            id(std::stoull(connection_state->getId())) {
        }
        Connection(const Connection& other) noexcept
          : Connection(other.web_socket, other.state) {
        }
        Connection(Connection&& other) noexcept
          : web_socket(std::move(other.web_socket)),
            state(std::move(other.state)), id(other.id) {};

        void operator=(const Connection& other) noexcept {
          web_socket = other.web_socket;
          state = other.state;
          id = other.id;
        }
        void operator=(Connection&& other) noexcept {
          web_socket = std::move(other.web_socket);
          state = std::move(other.state);
          id = std::exchange(other.id,
                             (std::numeric_limits<std::uint64_t>::max)());
        }
    };

    Server(int port, const std::string_view& host, Lobby& lobby,
           ConnectionClosureHandler& closure_handler);

    virtual void Start() override;

    virtual void End() override;

    void OnNewConnectionEstablished(std::weak_ptr<ix::WebSocket> web_socket,
                                    std::shared_ptr<ix::ConnectionState> state);

    void AddObserver(Observer* observer);

    void RemoveObserver(Observer* observer);

    void OnConnectionClosed(ix::ConnectionState* state);

  private:
    struct MessageHandler {
        std::weak_ptr<ix::WebSocket> web_socket{};
        std::shared_ptr<ix::ConnectionState> state{};
        Server* server{nullptr};
        ConnectionClosureHandler* closure_handler;

        void operator()(const ix::WebSocketMessagePtr& msg);
    };

    std::mutex observers_mutex_;
    std::vector<Observer*> observers_;

    std::unique_ptr<ix::WebSocketServer> server_;
    Lobby& lobby_;
    ConnectionClosureHandler& closure_handler_;

    common::utility::ScopedObservation<ServerManager, Server>
      server_manager_observation_;
};

} // namespace server

#endif // !SERVER_SERVER_H_
