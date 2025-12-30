set(SOURCE_FILES
    main.cc
    card.cc
    card.h
    deck.cc
    deck.h
    card_serializer.cc
    card_serializer.h
)

add_executable(server ${SOURCE_FILES})

target_include_directories(server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/common)

target_link_libraries(server PRIVATE
    ixwebsocket::ixwebsocket
    wsock32
    ws2_32
    Crypt32
)
