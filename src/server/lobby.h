#ifndef SERVER_LOBBY_H_
#define SERVER_LOBBY_H_

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdlib>
#include <deque>
#include <memory>
#include <mutex>
#include <print>
#include <ratio>
#include <utility>

#include "connection_closure_handler.h"
#include "scoped_observation.h"
#include "server.h"
#include "stacktrace_analyzer.h"

namespace server {

using namespace std::literals::chrono_literals;

class MatchMaker;

// Lobby implements a thread safe deque, but lobby uses it as a queue. I just
// need the arbitrary index access. Copy/Move constructor/assignements are
// deleted since I cannot foresee a need for them at the moment.
// Implements ConnectionClosureHandler::Observer to remove any closed
// connections stuck in the lobby.
class Lobby : public ConnectionClosureHandler::Observer {
  public:
    explicit Lobby(ConnectionClosureHandler& closure_handler)
      : closure_handler_observation_(this) {
      closure_handler_observation_.Observe(&closure_handler);
    }
    Lobby(const Lobby&) = delete;
    void operator=(const Lobby&) = delete;
    Lobby(Lobby&&) = delete;
    void operator=(Lobby&&) = delete;

    using Connection = server::Server::Connection;

    bool Empty() const {
      std::lock_guard lock{mutex_};
      return data_.empty();
    }

    void Push(std::shared_ptr<Server::Connection> value) {
      {
        std::lock_guard lock{mutex_};
        data_.push_back(std::move(value));
        std::print("Pushed. Size: {}\n", data_.size());
      }
      data_cv_.notify_one();
    }

    std::shared_ptr<Connection> TryPop() {
      std::lock_guard lock{mutex_};

      if (data_.empty()) {
        return nullptr;
      }

      std::shared_ptr<Connection> return_value = std::move(data_.front());
      data_.pop_front();
      return return_value;
    }

    std::shared_ptr<Connection>
    WaitPop(const std::chrono::duration<i32, std::ratio<1, 1>> wait_time = 1s) {
      std::unique_lock lock{mutex_};
      if (!data_cv_.wait_for(lock, wait_time, [&]() {
            return !data_.empty();
          })) {
        return nullptr;
      }

      std::shared_ptr<Connection> return_value = std::move(data_.front());
      data_.pop_front();
      return return_value;
    }

    virtual size_t OnConnectionClosed(u64 id) override {
      auto remove_lambda = [id](const std::shared_ptr<Connection>& connection) {
        if (!connection->closed.load()) {
          return false;
        }

        if (connection->id == id) {
          std::print("Connection {} erased from lobby\n", id);
          return true;
        } else {
          std::print("Connection was closed but the ids differ. Cooked!!!");
          common::utility::StacktraceAnalyzer::PrintOut();
          std::abort();
        }
      };

      std::lock_guard lock{mutex_};
      if (!data_.size()) {
        return 0;
      }
      return std::erase_if(data_, remove_lambda);
    }

  private:
    friend class MatchMaker;

    mutable std::mutex mutex_;
    std::condition_variable data_cv_;
    std::deque<std::shared_ptr<Connection>> data_;

    common::utility::ScopedObservation<ConnectionClosureHandler, Lobby>
      closure_handler_observation_;
};

} // namespace server

#endif // !SERVER_LOBBY_H_
