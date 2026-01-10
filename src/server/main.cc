#include "net/net_init_manager.h"
#include "server_manager.h"
#include "utility/stacktrace_analyzer.h"

int main() {
  common::utility::StacktraceAnalyzer::Initialize();
  common::net::NetInitManager::Initialize();
  server::ServerManager::Instance().Initialize();
  server::ServerManager::Instance().Start();
  server::ServerManager::Instance().Wait();
  server::ServerManager::Instance().End();
  return 0;
}
