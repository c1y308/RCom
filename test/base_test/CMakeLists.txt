# 为 base 模块里的各个头文件组件分别创建测试程序。
rcom_add_gtest(unbounded_queue_test
    unbounded_queue_test.cpp
)

# 测试程序需要包含 base 头文件，因此链接 RCom_base。
target_link_libraries(unbounded_queue_test PRIVATE
    RCom_base
)

rcom_add_gtest(bounded_queue_test
    bounded_queue_test.cpp
)
target_link_libraries(bounded_queue_test PRIVATE
    RCom_base
)

rcom_add_gtest(macros_test
    macros_test.cpp
)
target_link_libraries(macros_test PRIVATE
    RCom_base
)

rcom_add_gtest(atomic_rw_lock_test
    atomic_rw_lock_test.cpp
)
target_link_libraries(atomic_rw_lock_test PRIVATE
    RCom_base
)
