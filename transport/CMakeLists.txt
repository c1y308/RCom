add_library(RCom_transport SHARED
    common/endpoint.cpp
    common/identity.cpp

    message/message_info.cpp
)

target_include_directories(RCom_transport PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/message
)

target_link_libraries(RCom_transport
PUBLIC
    RCom_base
    RCom_config
    RCom_log
PRIVATE
    RCom_common
)

