cmake_minimum_required(VERSION 3.25)
project(2023 C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(2023
        main.c
        main.h
        tree_node.c
        tree_node.h)
