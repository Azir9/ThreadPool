cmake_minimum_required(VERSION 3.15)  # 指定 CMake 最低版本
project(MinToggleSteps)               # 项目名称

set(CMAKE_CXX_STANDARD 17)            # 使用 C++17 标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)   # 强制要求 C++17


add_library(ThreadPool SHARED threadpool.cpp)
# 添加可执行文件（假设主程序在 main.cpp）
target_include_directories(ThreadPool PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(test 
    test.cpp 
    threadpool.cpp)

target_link_libraries(test pthread)

