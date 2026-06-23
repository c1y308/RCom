# log 模块测试程序。
rcom_add_gtest(log_test
    log_test.cpp
)

# 链接 RCom_log，同时获得 logger.hpp/logstream.hpp 的头文件路径。
target_link_libraries(log_test PRIVATE
    RCom_log
)
