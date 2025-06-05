cmake_minimum_required(VERSION 3.29)
project(direct2d_rendertarget LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(direct2d_rendertarget WIN32 main.cpp
        base_window.h
        base_window.cpp
)