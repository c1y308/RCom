# time 模块由多个 .cpp 文件组成，因此创建动态库目标。
add_library(RCom_time SHARED
            duration.cpp
            rate.cpp
            time.cpp)

# PUBLIC 表示 RCom_time 自己和链接它的目标都能使用该头文件目录。
target_include_directories(RCom_time PUBLIC
                            ${CMAKE_CURRENT_SOURCE_DIR})
