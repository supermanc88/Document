cmake_minimum_required(VERSION 3.7)
project(fifo)

set(CMAKE_CXX_STANDARD 11)

add_executable(fifo_w fifo_write.cpp)

add_executable(fifo_r fifo_read.cpp)