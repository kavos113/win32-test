cmake_minimum_required(VERSION 3.29)
project(exception CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../../bin/exception)

add_library(exception SHARED exception.cpp)
target_include_directories(exception PRIVATE $ENV{JAVA_HOME}/include $ENV{JAVA_HOME}/include/win32)