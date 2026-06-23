# base 目录目前都是头文件模板/内联实现，因此使用 INTERFACE 库。
add_library(RCom_base INTERFACE)

# INTERFACE include 路径会传递给链接 RCom_base 的目标。
target_include_directories(RCom_base INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
