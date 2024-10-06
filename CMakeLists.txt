cmake_minimum_required(VERSION 3.29)
project(win32_test CXX)

file(GLOB children RELATIVE ${CMAKE_SOURCE_DIR} */CMakeLists.txt)

foreach (child ${children})
    get_filename_component(dir ${child} DIRECTORY)
    add_subdirectory(${dir})
endforeach ()