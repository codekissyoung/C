cmake_minimum_required(VERSION 3.15)

project(2020 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

add_executable(2020 main.c ../unixDomainSocket/main.c ../unixDomainSocket/main.h)
