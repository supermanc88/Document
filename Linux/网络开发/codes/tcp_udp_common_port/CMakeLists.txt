cmake_minimum_required(VERSION 3.7)
project(tcp_udp_oneport)

set(CMAKE_CXX_STANDARD 11)

add_executable(tcp_udp_oneport main.cpp)
target_link_libraries(tcp_udp_oneport pthread)