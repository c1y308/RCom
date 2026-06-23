# time 模块测试程序，覆盖 Duration/Time/Rate。
rcom_add_gtest(time_test
    time_test.cpp
)

# 链接 RCom_time，同时获得 time 模块头文件路径。
target_link_libraries(time_test PRIVATE
    RCom_time
)
