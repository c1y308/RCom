# common 目前也是头文件工具集合，因此使用 INTERFACE 库。
add_library(RCom_common INTERFACE)

# 暴露 common 目录，方便使用 #include "macros.hpp"。
target_include_directories(RCom_common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# common/macros.hpp 依赖 base/macros.hpp，所以把依赖关系传递出去。
target_link_libraries(RCom_common INTERFACE RCom_base)
