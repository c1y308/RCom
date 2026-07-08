add_library(RCom_config INTERFACE)

target_include_directories(RCom_config INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(RCom_config INTERFACE
    RCom_serialize
    nlohmann_json::nlohmann_json
)
