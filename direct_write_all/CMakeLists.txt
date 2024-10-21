cmake_minimum_required(VERSION 3.29)
project(direct_write_all CXX)

add_executable(direct_write_all WIN32 base_window.h main_window.cpp
        hello_text.cpp)