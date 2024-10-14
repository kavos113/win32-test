cmake_minimum_required(VERSION 3.29)
project(window_test CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../../bin/window_test)

add_library(window_test SHARED window_WindowTest.cpp)
target_include_directories(window_test PUBLIC $ENV{JAVA_HOME}/include $ENV{JAVA_HOME}/include/win32)