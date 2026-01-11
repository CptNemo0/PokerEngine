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
        // Ok so right now whenever somebody disconnects this is called. Class
        // implementing this interface should somehow deal with this. However I
        // feel like this is not enough. Because I can imagine a scenario where
        // when this is called a connection concerning a disconnected player is
        // moved and the fact that somebody disconnected might get lost in
        // translation. What I've been thinking is to decouple this from a
        // function and just set an atomic bool, that can be later checked in
        // crucial places in code as many times as I want.
        //
        // Second thought would be to have a shared queue and whenever it's not
        // empty, implementing classes be trying to check for the disconnected
        // player in their "stashes". Once they find the player, it will be
        // removed from the queue and next time somebody will check it the queue
        // will be empty, so this way threads would communicated between each
        // other that "Hey I found that rascal, don't look for it anymore."
        // Maybe the closure handler can have api for this queue.
        virtual size_t OnConnectionClosed(u64 id) = 0;
    };

    void OnConnectionClosed(u64 id);

    void AddObserver(Observer* observer);
    void RemoveObserver(Observer* observer);

    std::mutex observers_mutex_;
    std::vector<Observer*> observers_;
};

} // namespace server

#endif // !SERVER_CONNECTION_CLOSURE_HANDLER_H_
