cmake_minimum_required(VERSION 3.29)
project(hello_world LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(hello_world WIN32 main.cpp)