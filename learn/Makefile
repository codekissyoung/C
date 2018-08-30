cky:lib/func.o lib/node.o lib/stdarg.o lib/main.o lib/thread.o lib/global_variables.o lib/libshare.so lib/libstatic.a
	gcc -Q -g -std=c11 -pedantic -Wall -Werror -Iinclude -Llib -lstatic -lshare -pthread lib/func.o lib/node.o lib/stdarg.o lib/thread.o lib/global_variables.o lib/main.o -o cky

# $< 表示依赖的第一个文件
# $^ 表示依赖的所有文件
# $@ 表示目标文件

lib/main.o:main.c
	gcc -Q -c -Iinclude $< -o $@

lib/global_variables.o:src/global_variables.c
	gcc -Q -c -Iinclude $< -o $@

lib/thread.o:src/thread.c
	gcc -Q -c -Iinclude $< -o $@

lib/static.o:src/static.c
	gcc -Q -c -Iinclude $< -o $@

lib/node.o:src/node.c
	gcc -Q -c -Iinclude $< -o $@

lib/func.o:src/func.c
	gcc -Q -c -Iinclude $< -o $@

lib/stdarg.o:src/stdarg.c
	gcc -Q -c -Iinclude $< -o $@

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
