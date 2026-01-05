set(SOURCE_FILES
    model/card.cc
    model/card.h
    utility/card_serializer.cc
    utility/card_serializer.h
    utility/sorted_vector.h
    utility/enum_indexable_array.h
    net/net_init_manager.h
)

add_library(common STATIC ${SOURCE_FILES})

target_include_directories(common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/model
    ${CMAKE_CURRENT_SOURCE_DIR}/utility)

target_link_libraries(common PRIVATE
    ixwebsocket::ixwebsocket
    wsock32
    ws2_32
    Crypt32
)
