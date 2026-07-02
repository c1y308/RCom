add_library(RCom_transport_commom SHARED
    endpoint.cpp
    identity.cpp
)

target_include_directories(RCom_transport_commom PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(RCom_transport_commom PRIVATE
    RCom_common
)
