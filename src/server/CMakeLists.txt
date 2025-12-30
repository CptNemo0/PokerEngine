add_executable(server main.cc)

target_link_libraries(server PRIVATE
    ixwebsocket::ixwebsocket
    wsock32
    ws2_32
    Crypt32
)
