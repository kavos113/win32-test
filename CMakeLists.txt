cmake_minimum_required(VERSION 3.29)
project(win32_test LANGUAGES C CXX)

file(GLOB_RECURSE children RELATIVE ${CMAKE_SOURCE_DIR} */CMakeLists.txt)

foreach (child ${children})
    get_filename_component(dir ${child} DIRECTORY)
    string(FIND ${dir} "out" pos)
    if (NOT ${pos} EQUAL -1)
        continue()
    endif ()
    if (EXISTS ${CMAKE_SOURCE_DIR}/${dir}/CMakeLists.txt)
        message(STATUS "Adding subdirectory ${CMAKE_SOURCE_DIR}/${dir}")
        add_subdirectory(${dir})
    endif ()
endforeach ()