#ifndef SERVER_LOBBY_H_
#define SERVER_LOBBY_H_

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <print>
#include <queue>
#include <ratio>
#include <utility>

#include "server.h"
#include "stacktrace_analyzer.h"

namespace server {

using namespace std::literals::chrono_literals;

class MatchMaker;

// Lobby implements a thread safe queue - that will probably be converted into a
// deque/vector for the erase() api. Copy/Move constructor/assignements are
// deleted since I cannot foresee a need for them at the moment.
class Lobby {
  public:
    Lobby() = default;
    Lobby(const Lobby&) = delete;
    void operator=(const Lobby&) = delete;
    Lobby(Lobby&&) = delete;
    void operator=(Lobby&&) = delete;

    using Connection = server::Server::Connection;

    bool Empty() const {
      std::lock_guard lock{mutex_};
      return data_.empty();
    }

    void Push(const Connection& value) {
      common::utility::StacktraceAnalyzer::PrintOut();
      {
        std::lock_guard lock{mutex_};
        data_.push(value);
        std::print("Pushed. Size: {}\n", data_.size());
      }
      data_cv_.notify_one();
    }

    void Push(Connection&& value) {
      {
        std::lock_guard lock{mutex_};
        data_.push(std::move(value));
        std::print("Moved. Size: {}\n", data_.size());
      }
      data_cv_.notify_one();
    }

    bool TryPop(Connection& result) {
      std::lock_guard lock{mutex_};
      if (data_.empty()) {
        return false;
      }

      // Moving the data out, since the queue will pop it anyway.
      result = std::move(this->data_.front());

      data_.pop();
      return true;
    }

    std::unique_ptr<Connection> TryPop() {
      std::lock_guard lock{mutex_};

      if (data_.empty()) {
        return nullptr;
      }

      // Moving the data out, since the queue will pop it anyway.
      std::unique_ptr<Connection> return_value =
        std::make_unique<Connection>(std::move(data_.front()));

      data_.pop();
      return return_value;
    }

    bool
    WaitPop(server::Server::Connection& result,
            const std::chrono::duration<i32, std::ratio<1, 1>> wait_time = 1s) {
      std::unique_lock lock{mutex_};
      if (!data_cv_.wait_for(lock, wait_time, [&]() {
            return !data_.empty();
          })) {
        return false;
      }

      // Moving the data out, since the queue will pop it anyway.
      result = std::move(this->data_.front());

      data_.pop();
      return true;
    }

    std::unique_ptr<Connection>
    WaitPop(const std::chrono::duration<i32, std::ratio<1, 1>> wait_time = 1s) {
      std::unique_lock lock{mutex_};

      if (!data_cv_.wait_for(lock, wait_time, [&]() {
            return !data_.empty();
          })) {
        return nullptr;
      }

      // Moving the data out, since the queue will pop it anyway.
      std::unique_ptr<Connection> return_value =
        std::make_unique<Connection>(std::move(data_.front()));

      data_.pop();

      return return_value;
    }

  private:
    friend class MatchMaker;

    mutable std::mutex mutex_;
    std::condition_variable data_cv_;
    std::queue<Connection> data_;
};

} // namespace server

#endif // !SERVER_LOBBY_H_
