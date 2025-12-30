set(SOURCE_FILES
    main.cc
    model/card.cc
    model/card.h
    model/deck.cc
    model/deck.h
    utility/card_serializer.cc
    utility/card_serializer.h
)

add_executable(common ${SOURCE_FILES})

target_include_directories(common PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/model
    ${CMAKE_CURRENT_SOURCE_DIR}/utility
    ${CMAKE_SOURCE_DIR}/src/common)

target_link_libraries(common PRIVATE
    ixwebsocket::ixwebsocket
    wsock32
    ws2_32
    Crypt32
)
