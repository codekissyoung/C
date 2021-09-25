cmake_minimum_required(VERSION 3.20)
project(2021 C)
set(CMAKE_C_STANDARD 11)
# 构建时，将测试文件也拷贝过去
configure_file(data/test.bin data/test.bin)
add_executable(2021 main.c func.c func.h)
