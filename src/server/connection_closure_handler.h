#ifndef SERVER_CONNECTION_CLOSURE_HANDLER_H_
#define SERVER_CONNECTION_CLOSURE_HANDLER_H_

#include <cstddef>
#include <mutex>
#include <vector>

#include "aliasing.h"

namespace server {

// ConnectionClosureHandler is responsible for informing objects that "stash"
// connections about the disconnections. Interested classes implement
// ConnectionClosureHandler::Observer.
class ConnectionClosureHandler {
  public:
    class Observer {
      public:
        virtual size_t OnConnectionClosed(u64 id) = 0;
    };

    // Calls OnConnectionClosed on all Observers.
    void OnConnectionClosed(u64 id);

    void AddObserver(Observer* observer);
    void RemoveObserver(Observer* observer);

    std::mutex observers_mutex_;
    std::vector<Observer*> observers_;
};

} // namespace server

#endif // !SERVER_CONNECTION_CLOSURE_HANDLER_H_
