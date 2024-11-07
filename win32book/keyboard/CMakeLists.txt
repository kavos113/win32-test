cmake_minimum_required(VERSION 3.29)
project(win32book_keyboard C)

add_executable(win32book_keyboard WIN32 main.c)
target_link_libraries(win32book_keyboard PRIVATE winmm)