cmake_minimum_required(VERSION 3.29)
project(split_classes LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(split_classes WIN32 main.cpp
        base_window.h
)