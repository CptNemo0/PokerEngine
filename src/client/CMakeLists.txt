add_executable(client main.cc)

target_include_directories(client  PRIVATE
    ${CMAKE_SOURCE_DIR}/src/common)

target_link_libraries(client PRIVATE
    common
    ixwebsocket::ixwebsocket
    wsock32
    ws2_32
    Crypt32
)
