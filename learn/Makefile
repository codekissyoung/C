# makefile for cky
# 使用 C11 标准编译
# -Llib 指定当前目录下的lib目录,系统的缺省库的路径/lib /usr/lib /usr/local/lib 在这三个路径下面的库,我们可以不指定路径
#  -pedantic 选项能够帮助程序员发现一些不符合 ANSI/ISO C标准的代码
cky:lib/func.o lib/node.o lib/stdarg.o lib/main.o lib/thread.o lib/global_variables.o lib/libshare.so lib/libstatic.a
	gcc -g -std=c11 -pedantic -Wall -Werror -Iinclude -Llib -lstatic -lshare -pthread lib/func.o lib/node.o lib/stdarg.o lib/thread.o lib/global_variables.o lib/main.o -o cky

# $< 表示依赖的第一个文件
# $^ 表示依赖的所有文件
# $@ 表示目标文件

lib/main.o:main.c
	gcc -c -Iinclude $< -o $@

lib/global_variables.o:src/global_variables.c
	gcc -c -Iinclude $< -o $@

lib/thread.o:src/thread.c
	gcc -c -Iinclude $< -o $@

lib/static.o:src/static.c
	gcc -c -Iinclude $< -o $@

lib/node.o:src/node.c
	gcc -c -Iinclude $< -o $@

lib/func.o:src/func.c
	gcc -c -Iinclude $< -o $@

lib/stdarg.o:src/stdarg.c
	gcc -c -Iinclude $< -o $@

# 静态库的编译
lib/libstatic.a:lib/static.o
	ar rcs $@ $<

# 动态库的编译
lib/libshare.so:src/share.c
	gcc -shared -fPIC -Iinclude $< -o $@

clean:
	rm lib/*.o
	rm lib/*.so
	rm lib/*.a
	rm cky
