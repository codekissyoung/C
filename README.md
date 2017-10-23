# cky
自己在linux下c编程的一个工具

# gcc 编译命令
```shell
gcc -c main.c -o main.o # 编译成目标文件
gcc -I dir ...          # 添加 #include 'xxx.h' 搜索目录
```

# 编译器警告设置
```shell
-Wall # 启用所有警告
-Werror # 发生警告时 退出编译
-g # 支持 gdb 调试
```

# 编译优化
```shell
-O
-O2
-O3
-finline-functions  # 简单函数可以在其调用处展开
-funswitch-loops    # 将循环体内不改变的变量移到循环体外
```

# 编译
```
make
```

# 运行
```
./cky -a -b -c
```

# 重新编译
```
make clean #清除上次编译的内容
make
```
