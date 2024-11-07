cmake_minimum_required(VERSION 3.29)
project(win32book_bitmap C)

add_executable(win32book_bitmap WIN32 main.c main.rc)
target_link_libraries(win32book_bitmap PRIVATE winmm)