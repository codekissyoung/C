#!/bin/bash

echo "内核版本:"
uname -a; echo;                             # 检测 linux 内核版本

echo "发行版本:";
lsb_release -a; echo;                       # 查看发行版本

echo "gcc版本:"
gcc -v  | grep "gcc version"; echo;         # 检查 gcc 版本

echo "gdb版本:";
gdb -v  | grep -i "GNU gdb";echo;           # 检查 gdb 版本

echo "make版本:";
make -v | grep -i make;echo;                # 检查 make 版本

echo "autoconf版本:";
autoconf --version | grep autoconf;echo;    # 检查 autoconf 版本

echo "automake版本:";
automake --version | grep automake;echo;    # 检查 automake 版本

echo "libtoolize版本";
libtoolize --version | grep automake;echo;  # 检查 libtool 版本

echo "cmake版本";
cmake --version ;echo;                      # 检查 cmake 版本


