# common 模块当前只需要测试 macros.hpp。
rcom_add_gtest(common_macros_test
    macros_test.cpp
)

# 链接 RCom_common 后会自动获得 common 和 base 的头文件路径。
target_link_libraries(common_macros_test PRIVATE
    RCom_common
)
