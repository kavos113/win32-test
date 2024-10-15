cmake_minimum_required(VERSION 3.29)
project(window_java CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../../bin/window_java)

add_library(window_java SHARED window_test.cpp
        java_component.cpp
        java_component.h
        java_button.cpp
        java_button.h
        java_window.cpp
        java_window.h
        util.cpp
        util.h)
target_include_directories(window_java PUBLIC $ENV{JAVA_HOME}/include $ENV{JAVA_HOME}/include/win32)