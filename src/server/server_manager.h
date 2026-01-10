#ifndef SERVER_SERVER_MANAGER_H_
#define SERVER_SERVER_MANAGER_H_

#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <print>
#include <string>
#include <vector>

namespace server {

class Lobby;
class MatchMaker;
class Server;

class ServerManager {
  public:
    class Observer {
      public:
        // Observer should not try to remove itself from the observers_
        // collection of the ServerManager in those functions. ServerManager
        // iterates over this collection when calling those functions so it
        // would probably mess up the iterators.
        //
        // Called after the initialization. Assumes that the object is
        // initialized and ready to start working. Fix und fertig.
        virtual void Start() = 0;
        // Called before the program exits. After it's called, program assumes
        // that all threads connected to this object have been joined.
        virtual void End() = 0;
    };

    // This is a singleton.
    ServerManager(const ServerManager&) = delete;
    ServerManager(ServerManager&&) = delete;
    ServerManager* operator=(const ServerManager&) = delete;
    ServerManager* operator=(ServerManager&&) = delete;
    // This is a singleton.

    // Adds a new observer to the observers_ collection.
    void AddObserver(Observer* observer);

    // Removes an observer from the observers_ collection.
    void RemoveObserver(Observer* observer);

    // Since some of the member sub-objects of the ServerManager call
    // ServerManager::Instance() in their constructors, ServerManager must
    // deffer their initialization to this Initialize() function to avoid a
    // deadlock - this might (and will) happen since a first call to Instance()
    // will call the constructor of `instance` static variable. Were the
    // sub-objects initialized in the constructor of ServerManager, some of them
    // would try to call Instance() and a thread would block since a constructor
    // did not finish.
    void Initialize();

    // Calls Start() method of all observers effectively starting the server,
    void Start();

    // Awaits for user input "q" or "quit" that will result in program's
    // shutdown
    void Wait();

    // Calls the End() method of all observers
    void End();

    static ServerManager& Instance() {
      std::print("Instance\n");
      static ServerManager instance;
      return instance;
    }

    ~ServerManager();

  private:
    // Class Waiter waits for the input to close the program.
    class Waiter {
      public:
        void Listen() {
          std::string line;
          while (std::getline(std::cin, line)) {
            if (line == "q" || line == "quit") {
              ServerManager::Instance().wait_cv_.notify_all();
              break;
            }
          }
        }
    };

    ServerManager();

    std::unique_ptr<Lobby> lobby_{nullptr};
    std::unique_ptr<Server> server_{nullptr};
    std::unique_ptr<MatchMaker> match_maker_{nullptr};

    mutable std::mutex observers_mutex_;
    std::vector<Observer*> observers_;

    mutable std::mutex wait_mutex_;
    std::condition_variable wait_cv_;
};

} // namespace server

#endif // !SERVER_SERVER_MANAGER_H_
