cmake_minimum_required(VERSION 3.29)
project(direct2d_image LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(direct2d_image WIN32 main.cpp
        base_window.h
        base_window.cpp
        main_window.cpp
        main_window.h
)