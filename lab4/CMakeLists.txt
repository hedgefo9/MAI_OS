cmake_minimum_required(VERSION 3.24)
project(os_lab2)

set(CMAKE_CXX_STANDARD 17)

add_library(lib1 lib1.cpp)
add_library(lib2 lib2.cpp)
add_executable(os_lab4 main.cpp lib2)
add_executable(os_lab4_2 main2.cpp)
