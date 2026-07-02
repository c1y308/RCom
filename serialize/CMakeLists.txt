add_library(RCom_serialize SHARED
    data_stream.cpp
)

target_include_directories(RCom_serialize PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
