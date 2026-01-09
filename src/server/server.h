#ifndef SERVER_SERVER_H_
#define SERVER_SERVER_H_

#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "ixwebsocket/IXConnectionState.h"
#include "ixwebsocket/IXWebSocket.h"
#include "ixwebsocket/IXWebSocketMessage.h"
#include "ixwebsocket/IXWebSocketServer.h"

#include "aliasing.h"
#include "stacktrace_analyzer.h"

namespace server {

class Lobby;

class Server {
  public:
    struct Connection {
        std::weak_ptr<ix::WebSocket> web_socket{};
        std::shared_ptr<ix::ConnectionState> state{};
        std::uint64_t id{(std::numeric_limits<std::uint64_t>::max)()};

        Connection() = default;

        Connection(std::weak_ptr<ix::WebSocket> socket,
                   std::shared_ptr<ix::ConnectionState> connection_state)
          : web_socket(socket), state(connection_state),
            id(std::stoull(connection_state->getId())) {
          common::utility::StacktraceAnalyzer::PrintOut();
        }
        Connection(const Connection& other) noexcept
          : Connection(other.web_socket, other.state) {
          common::utility::StacktraceAnalyzer::PrintOut();
        }
        Connection(Connection&& other) noexcept
          : web_socket(std::move(other.web_socket)),
            state(std::move(other.state)), id(other.id) {
          common::utility::StacktraceAnalyzer::PrintOut();
        };

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

    Server(int port, const std::string_view& host, Lobby& lobby);

    void Start();

    void Wait() {
      server_->wait();
    }

    void Stop() {
      server_->stop();
    }

    void OnNewConnectionEstablished(std::weak_ptr<ix::WebSocket> web_socket,
                                    std::shared_ptr<ix::ConnectionState> state);

    // void OnConnectionClosed(const Connection& connection) {};
    //
    // void OnMessageReceived(const std::string& str,
    //                        const Connection& connection) {};

  private:
    struct MessageHandler {
        std::weak_ptr<ix::WebSocket> web_socket{};
        std::shared_ptr<ix::ConnectionState> state{};
        Server* server{nullptr};

        void operator()(const ix::WebSocketMessagePtr& msg);
    };

    std::unique_ptr<ix::WebSocketServer> server_;
    Lobby& lobby_;
};

} // namespace server

#endif // !SERVER_SERVER_H_
