# FetchContent 可以在配置阶段下载/引入第三方 CMake 项目。
include(FetchContent)

# 引入 GoogleTest，并把 gtest_main 链接到测试程序。
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)

# 封装一个小函数，减少每个测试目标重复写 add_executable 和 gtest_main。
function(rcom_add_gtest target)
    add_executable(${target} ${ARGN})
    
    target_link_libraries(${target} PRIVATE gtest_main)
endfunction()

# 每个测试子目录对应一个被测试模块。
add_subdirectory(base_test)
add_subdirectory(common_test)
add_subdirectory(log_test)
add_subdirectory(time_test)
add_subdirectory(serialize_test)
