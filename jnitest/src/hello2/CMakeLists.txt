cmake_minimum_required(VERSION 3.29)
project(hello2 CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../../bin/hello2)

add_library(hello2 SHARED hello2_HelloJNI.cpp)
target_include_directories(hello2 PUBLIC $ENV{JAVA_HOME}/include $ENV{JAVA_HOME}/include/win32)