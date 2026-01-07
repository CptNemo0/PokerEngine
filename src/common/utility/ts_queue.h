#ifndef COMMON_UTILIY_TS_QUEUE_H_
#define COMMON_UTILIY_TS_QUEUE_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <type_traits>

namespace common::utility {

template <class T>
concept ThreadSafeQueueElement = std::is_nothrow_copy_assignable_v<T> &&
                                 std::is_nothrow_copy_constructible_v<T>;

// Thread-safe queue implementation
template <ThreadSafeQueueElement T> class ts_queue {
  public:
    ts_queue() = default;
    ts_queue(const ts_queue&) = delete;
    void operator=(const ts_queue&) = delete;
    ts_queue(ts_queue&&) = delete;
    void operator=(ts_queue&&) = delete;

    bool empty() const {
      std::lock_guard lock{mutex_};
      return data_.empty();
    }

    void push(T value) {
      {
        std::lock_guard lock{mutex_};
        data_.push(value);
      }
      data_cv_.notify_one();
    }

    bool try_pop(T& result) {
      std::lock_guard lock{mutex_};
      if (data_.empty()) {
        return false;
      }

      result = [this] {
        if constexpr (std::is_nothrow_move_assignable_v<T>) {
          return std::move(this->data_.front());
        } else {
          return (this->data_.front());
        }
      }();

      data_.pop();
      return true;
    }

    std::unique_ptr<T> try_pop() {
      std::lock_guard lock{mutex_};

      if (data_.empty()) {
        return nullptr;
      }

      std::unique_ptr<T> return_value = [this] {
        if constexpr (std::is_nothrow_move_constructible_v<T>) {
          return std::make_unique<T>(std::move(data_.front()));
        } else {
          return std::make_unique<T>(data_.front());
        }
      }();

      data_.pop();
      return return_value;
    }

    void wait_pop(T& result) {
      std::unique_lock lock{mutex_};
      data_cv_.wait(lock, [&]() {
        return !data_.empty();
      });

      result = [this] {
        if constexpr (std::is_nothrow_move_assignable_v<T>) {
          return std::move(this->data_.front());
        } else {
          return (this->data_.front());
        }
      }();
      data_.pop();
    }

    std::unique_ptr<T> wait_pop() {
      std::lock_guard lock{mutex_};
      data_cv_.wait(lock, [&]() {
        return !data_.empty();
      });

      std::unique_ptr<T> return_value = [this] {
        if constexpr (std::is_nothrow_move_constructible_v<T>) {
          return std::make_unique<T>(std::move(data_.front()));
        } else {
          return std::make_unique<T>(data_.front());
        }
      }();

      data_.pop();
      return return_value;
    }

  private:
    mutable std::mutex mutex_;
    std::condition_variable data_cv_;
    std::queue<T> data_;
};

} // namespace common::utility

#endif // !COMMON_UTILIY_TS_QUEUE_H_
