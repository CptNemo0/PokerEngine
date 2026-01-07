#ifndef SERVER_LOBBY_H_
#define SERVER_LOBBY_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <print>
#include <queue>
#include <utility>

#include "server.h"

namespace server {

class GameAssembler;

class Lobby {
  public:
    Lobby() = default;
    Lobby(const Lobby&) = delete;
    void operator=(const Lobby&) = delete;
    Lobby(Lobby&&) = delete;
    void operator=(Lobby&&) = delete;

    using Connection = server::Server::Connection;
    using reference = server::Server::Connection&;
    using const_reference = const server::Server::Connection&;

    bool Empty() const {
      std::lock_guard lock{mutex_};
      return data_.empty();
    }

    void Push(const_reference value) {
      {
        std::lock_guard lock{mutex_};
        data_.push(value);
        std::print("Pushed. Size: {}\n", data_.size());
      }
      data_cv_.notify_one();
    }

    bool TryPop(reference result) {
      std::lock_guard lock{mutex_};
      if (data_.empty()) {
        return false;
      }

      result = std::move(this->data_.front());

      data_.pop();
      return true;
    }

    std::unique_ptr<Connection> TryPop() {
      std::lock_guard lock{mutex_};

      if (data_.empty()) {
        return nullptr;
      }

      std::unique_ptr<Connection> return_value =
        std::make_unique<Connection>(std::move(data_.front()));

      data_.pop();
      return return_value;
    }

    void WaitPop(reference result) {
      std::unique_lock lock{mutex_};
      data_cv_.wait(lock, [&]() {
        return !data_.empty();
      });

      result = std::move(this->data_.front());
      data_.pop();
    }

    std::unique_ptr<Connection> WaitPop() {
      std::unique_lock lock{mutex_};
      data_cv_.wait(lock, [&]() {
        return !data_.empty();
      });

      std::unique_ptr<Connection> return_value =
        std::make_unique<Connection>(std::move(data_.front()));

      data_.pop();
      return return_value;
    }

  private:
    friend class GameAssembler;

    mutable std::mutex mutex_;
    std::condition_variable data_cv_;
    std::queue<server::Server::Connection> data_;
};

} // namespace server

#endif // !SERVER_LOBBY_H_
