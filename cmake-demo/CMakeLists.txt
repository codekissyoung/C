project(main)                           # project 命令 ,参数是 main ,该命令表示项目的名称是 main
cmake_minimum_required(VERSION 3.0)     # 限定了 CMake 的版本

add_subdirectory(mod1)
add_subdirectory(mod2)

# 查找指定目录(.)下的所有源文件，然后将结果存进指定变量名
aux_source_directory(. DIR_SRCS)

# 将DIR_SRCS 中的源文件需要编译 成一个名称为 main 的可执行文件
add_executable(server ${DIR_SRCS} common.cpp common.h)

# 指明可执行文件 main 需要连接mod1.so 和mod2.so两个库
target_link_libraries(server mod1 mod2)
