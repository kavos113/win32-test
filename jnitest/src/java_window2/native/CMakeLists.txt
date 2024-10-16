cmake_minimum_required(VERSION 3.29)
project(java_window2 CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../../../bin/java_window2)

add_library(java_window2 SHARED java_component.cpp
        java_window.cpp
        java_window.h)
target_include_directories(java_window2 PUBLIC $ENV{JAVA_HOME}/include $ENV{JAVA_HOME}/include/win32)