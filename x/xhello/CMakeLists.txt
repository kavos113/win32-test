cmake_minimum_required(VERSION 3.29)
project(xhello C)

add_executable(xhello xhello.c)
target_include_directories(java_window2 PUBLIC $ENV{JAVA_HOME}/include $ENV{JAVA_HOME}/include/win32)