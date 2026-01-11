set(SOURCE_FILES
    main.cc
    server.cc
    server.h
    lobby.h
    match_maker.cc
    match_maker.h
    match_conductor.cc
    match_conductor.h
    server_constants.h
    server_manager.cc
    server_manager.h
    connection_closure_handler.cc
    connection_closure_handler.h
    match_conductor_manager.cc
    match_conductor_manager.h
    model/deck.cc
    model/deck.h
    model/hand_evaluator.cc
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
    dbghelp
)

target_compile_options(server PRIVATE
    $<$<CONFIG:Debug>:
        -g
        -O0
        -DDEBUG_MODE
    >
    $<$<CONFIG:Release>:
        -O3
        -DNDEBUG
    >
)
