cmake_minimum_required(VERSION 3.29)
project(array CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../../bin/array)

add_library(array SHARED array.cpp)
target_include_directories(array PRIVATE $ENV{JAVA_HOME}/include $ENV{JAVA_HOME}/include/win32)