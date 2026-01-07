set(SOURCE_FILES
    main.cc
    server.cc
    server.h
    lobby.h
    game_assembler.cc
    game_assembler.h
    server_constants.h
    model/deck.cc
    model/deck.h
    model/hand_evaluator.cc
    model/hand_evaluator.h
    model/hand_evaluator.h
)

add_executable(server ${SOURCE_FILES})

target_include_directories(server  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/model
    ${CMAKE_SOURCE_DIR}/src/common)

target_link_libraries(server PRIVATE
    common
    ixwebsocket::ixwebsocket
    wsock32
    ws2_32
    Crypt32
)
