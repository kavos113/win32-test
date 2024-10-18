cmake_minimum_required(VERSION 3.29)
project(access CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../../bin/access)

add_library(access SHARED access.cpp)
target_include_directories(access PRIVATE $ENV{JAVA_HOME}/include $ENV{JAVA_HOME}/include/win32)