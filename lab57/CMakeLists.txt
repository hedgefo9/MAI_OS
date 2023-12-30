cmake_minimum_required(VERSION 3.24)
project(os_lab57)

set(CMAKE_CXX_STANDARD 17)

add_executable(node node.cpp)
add_executable(os_lab57 main.cpp)

find_package(cppzmq)
target_link_libraries(os_lab57 cppzmq)
target_link_libraries(node cppzmq)

