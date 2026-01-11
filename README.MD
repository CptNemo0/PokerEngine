# Poker Server + Clients

## About this repository

Hereby repository contains the source code of poker "engine-server" and poker "agent-client".

## Build

Dependencies:

- zlib
- OpenSSL
- Mbed TLS
- dbghelp.lib

IXWebsocket will be installed via CMake's FetchContent. On Windows (I don't know about other platforms) IXWebsocket has some problems with mbedtls. To solve it, paste those two includes to IXSocketMbedTLS.cpp file.

```
#include <mbedtls/version.h>
#include <psa/crypto.h>
```
