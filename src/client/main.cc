#include "ixwebsocket/IXWebSocketMessage.h"
#include "ixwebsocket/IXWebSocketMessageType.h"
#include <cstdlib>
#include <ctime>
#include <format>
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>

#include <iostream>
#include <string>

#include "net/net_init_manager.h"

int main() {
  srand(time(0uz) * 100.0f);
  common::net::NetInitManager::Initialize();
  ix::WebSocket webSocket;
  std::cout << "Creating websocket\n";
  std::string url = std::format("ws://localhost:8008/user-{}", rand());
  webSocket.setUrl(url);

  // Optional heart beat, sent every 45 seconds when there is not any traffic
  // to make sure that load balancers do not kill an idle connection.
  webSocket.setPingInterval(45);

  // Per message deflate connection is not enabled by default. You can tweak its
  // parameters, enable or disable it with
  webSocket.enablePerMessageDeflate();
  webSocket.disablePerMessageDeflate();

  // Setup a callback to be fired when a message or an event (open, close,
  // error) is received
  webSocket.setOnMessageCallback([](const ix::WebSocketMessagePtr& msg) {
    if (msg->type == ix::WebSocketMessageType::Message) {
      std::cout << msg->str << std::endl;
    }
  });

  // Now that our callback is setup, we can start our background thread and
  // receive messages
  webSocket.start();

  char a = 'z';
  while (a != 'q') {
    std::cin >> a;
  }

  // Send a message to the server (default to TEXT mode)
  webSocket.send("hello world");

  // The message can be sent in BINARY mode (useful if you send MsgPack data for
  // example)
  webSocket.sendBinary("some serialized binary data");

  a = 'z';
  while (a != 'q') {
    std::cin >> a;
  }

  // Stop the connection
  webSocket.stop();
  return 0;
}
