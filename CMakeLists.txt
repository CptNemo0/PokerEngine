cmake_minimum_required(VERSION 4.0)
include(FetchContent)

project (Poker VERSION 1.0 DESCRIPTION "Poker server and clients" LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

FetchContent_Declare(
    ixwebsocket
    GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
    GIT_TAG        v11.4.6
)
FetchContent_MakeAvailable(ixwebsocket)

add_subdirectory(src)
