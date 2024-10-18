cmake_minimum_required(VERSION 3.29)
project(extend CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../../bin/extend)

add_library(extend SHARED extend.cpp)
target_include_directories(extend PRIVATE $ENV{JAVA_HOME}/include $ENV{JAVA_HOME}/include/win32)