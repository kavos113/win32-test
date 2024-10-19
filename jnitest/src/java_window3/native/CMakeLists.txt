cmake_minimum_required(VERSION 3.29)
project(java_window3 CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../../../bin/java_window3)

add_library(java_window3 SHARED java_component.cpp
        java_window.cpp
        java_window.h
        util.cpp
        java_button.cpp
        java_button.h)
target_include_directories(java_window3 PRIVATE $ENV{JAVA_HOME}/include $ENV{JAVA_HOME}/include/win32)