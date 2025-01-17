cmake_minimum_required(VERSION 3.29)
project(xaudio-3d CXX)

add_executable(xaudio-3d main.cpp)
target_link_libraries(xaudio-3d xaudio2.lib)
target_link_libraries(xaudio-3d winmm.lib)