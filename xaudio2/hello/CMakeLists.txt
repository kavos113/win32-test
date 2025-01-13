cmake_minimum_required(VERSION 3.29)
project(xaudio-hello CXX)

add_executable(xaudio-hello main.cpp)
target_link_libraries(xaudio-hello xaudio2.lib)
target_link_libraries(xaudio-hello winmm.lib)