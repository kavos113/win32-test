cmake_minimum_required(VERSION 3.29)
project(hello_win32 C)

add_executable(hello_win32 WIN32 hello_win.c
        BaseWindow.h)