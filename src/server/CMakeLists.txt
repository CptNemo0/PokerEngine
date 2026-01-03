set(SOURCE_FILES
    main.cc
    model/deck.cc
    model/deck.h
    logic/hand_evaluator.cc
    logic/hand_evaluator.h
)

add_executable(server ${SOURCE_FILES})

target_include_directories(server  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/model
    ${CMAKE_CURRENT_SOURCE_DIR}/logic
    ${CMAKE_SOURCE_DIR}/src/common)

target_link_libraries(server PRIVATE
    common
    ixwebsocket::ixwebsocket
    wsock32
    ws2_32
    Crypt32
)
