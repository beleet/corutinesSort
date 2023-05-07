cmake_minimum_required(VERSION 3.24)
project(mergeSortCorutines C)

set(CMAKE_C_STANDARD 11)

add_executable(mergeSortCorutines main.c libcoro.c solution.c)
