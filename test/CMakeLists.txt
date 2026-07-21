cmake_minimum_required(VERSION 3.14)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(RComTests LANGUAGES CXX)

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

if(NOT TARGET RCom_base)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../base" "${CMAKE_CURRENT_BINARY_DIR}/rcom_base")
endif()

if(NOT TARGET RCom_common)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../common" "${CMAKE_CURRENT_BINARY_DIR}/rcom_common")
endif()

if(NOT TARGET RCom_log)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../log" "${CMAKE_CURRENT_BINARY_DIR}/rcom_log")
endif()

if(NOT TARGET RCom_time)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../time" "${CMAKE_CURRENT_BINARY_DIR}/rcom_time")
endif()

if(NOT TARGET RCom_serialize)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../serialize" "${CMAKE_CURRENT_BINARY_DIR}/rcom_serialize")
endif()

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
add_subdirectory(singal_test)
add_subdirectory(common_test)
add_subdirectory(log_test)
add_subdirectory(time_test)
add_subdirectory(serialize_test)
