cmake_minimum_required(VERSION 3.10)

project(LauncherProject VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 使用 WIN32 参数将程序编译为 GUI 应用程序，从而不显示命令行窗口
add_executable(bg-laun WIN32 launcher.cpp)

set_target_properties(bg-laun PROPERTIES OUTPUT_NAME "bg-laun")
