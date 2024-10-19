cmake_minimum_required(VERSION 3.29)
project(direct_write_multistyle CXX)

add_executable(direct_write_multistyle WIN32 base_window.h
        hello_text.cpp
        hello_text.h
        main.cpp
        multi_style_text.cpp
        multi_style_text.h)
