#ifndef COMMON_NET_NET_INIT_MANGET_H
#define COMMON_NET_NET_INIT_MANGET_H

#include <ixwebsocket/IXNetSystem.h>

namespace common::net {
class NetInitManager {
  public:
    static void Initialize() {
      static NetInitManager instance{};
    }
    NetInitManager(const NetInitManager&) = delete;
    NetInitManager(NetInitManager&&) = delete;

    void operator=(const NetInitManager&) = delete;
    void operator=(NetInitManager&&) = delete;

    ~NetInitManager() {
      ix::uninitNetSystem();
    }

  private:
    NetInitManager() {
      ix::initNetSystem();
    }
};
} // namespace common::net

#endif // !COMMON_NET_NET_INIT_MANGET_H
