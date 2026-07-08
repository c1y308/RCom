cmake_minimum_required(VERSION 3.14)

# 定义项目名称、版本和使用的语言。
project(RCom VERSION 1.0.0 LANGUAGES CXX)

# 给 clangd / IDE 生成准确的编译数据库。
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 统一设置整个项目的 C++ 标准。
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# GCC/Clang 编译时打开常见警告，帮助尽早发现代码问题。
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 可选的运行时检查工具，默认关闭，需要时通过 -DENABLE_ASAN=ON 打开。
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(BUILD_GTESTS "Build GoogleTest test programs" OFF)

find_package(nlohmann_json REQUIRED)

if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()
if(ENABLE_TSAN)
    add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
    add_link_options(-fsanitize=thread)
endif()

# 添加各个功能模块。每个子目录负责定义自己的库目标。
add_subdirectory(base)
add_subdirectory(common)
add_subdirectory(log)
add_subdirectory(time)
add_subdirectory(serialize)
add_subdirectory(config)
add_subdirectory(transport)
# 按需构建 GoogleTest 测试程序。
if(BUILD_GTESTS)
    add_subdirectory(test)
endif()
