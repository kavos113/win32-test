cmake_minimum_required(VERSION 3.29)
project(win32_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(win32_test WIN32 main.cpp)