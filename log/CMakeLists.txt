# logger.cpp 有实际编译单元，因此创建动态库目标。
add_library(RCom_log SHARED
            logger.cpp)

# PUBLIC 表示 RCom_log 自己和链接它的目标都能使用该头文件目录。
target_include_directories(RCom_log PUBLIC
                            ${CMAKE_CURRENT_SOURCE_DIR})

# logger.hpp 依赖 common/macros.hpp，因此公开链接 RCom_common。
target_link_libraries(RCom_log PUBLIC
                      RCom_common)
