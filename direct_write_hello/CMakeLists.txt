cmake_minimum_required(VERSION 3.29)
project(direct_write_hello CXX)

add_executable(direct_write_hello WIN32 base_window.h
        hello_text.cpp
        hello_text.h
        main.cpp)
