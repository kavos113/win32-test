cmake_minimum_required(VERSION 3.29)
project(hello_jni CXX)

add_library(hello SHARED hello_Hello.cpp)
target_include_directories(hello PRIVATE $ENV{JAVA_HOME}/include $ENV{JAVA_HOME}/include/win32)