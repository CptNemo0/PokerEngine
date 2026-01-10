#include "connection_closure_handler.h"
#include "ixwebsocket/IXConnectionState.h"
#include <algorithm>
#include <mutex>

namespace server {

void ConnectionClosureHandler::AddObserver(Observer* observer) {
  std::lock_guard lock{observers_mutex_};
  observers_.push_back(observer);
}

void ConnectionClosureHandler::RemoveObserver(Observer* observer) {
  std::lock_guard lock{observers_mutex_};
  const auto result = std::erase(observers_, observer);
}

void ConnectionClosureHandler::OnConnectionClosed(ix::ConnectionState* state) {
  std::lock_guard lock{observers_mutex_};
  std::ranges::for_each(observers_, [state](Observer* observer) {
    if (observer->OnConnectionClosed(state)) {
      return;
    }
  });
}

} // namespace server
