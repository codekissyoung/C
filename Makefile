# makefile for cky
# 使用 C11 标准编译
# -Llib 指定当前目录下的lib目录,系统的缺省库的路径/lib /usr/lib /usr/local/lib 在这三个路径下面的库,我们可以不指定路径
#  -pedantic 选项能够帮助程序员发现一些不符合 ANSI/ISO C标准的代码
cky:lib/func.o lib/node.o lib/stdarg.o lib/main.o lib/thread.o lib/global_variables.o lib/share.so lib/libstatic_lib.a
	gcc -g -std=c11 -pedantic -Wall -Werror -Llib -lstatic_lib -pthread lib/func.o lib/node.o lib/stdarg.o lib/thread.o lib/global_variables.o lib/main.o lib/share.so  -o cky

# $< 表示依赖的第一个文件
# $^ 表示依赖的所有文件
# $@ 表示目标文件

lib/main.o:main.c
	gcc -c $< -o $@

lib/global_variables.o:src/global_variables.c
	gcc -c $< -o $@

lib/thread.o:src/thread.c
	gcc -c $< -o $@

lib/static_lib.o:src/static_lib.c
	gcc -c $< -o $@

lib/node.o:src/node.c
	gcc -c $< -o $@

lib/func.o:src/func.c
	gcc -c $< -o $@

lib/stdarg.o:src/stdarg.c
	gcc -c $< -o $@

# 静态库的编译
lib/libstatic_lib.a:lib/static_lib.o
	ar rcs $@ $<

# 动态库的编译
lib/share.so:src/share.c
	gcc -shared -fPIC $< -o $@

clean:
	rm lib/*.o
	rm lib/*.so
	rm lib/*.a
	rm cky









