# makefile for cky
cky:lib/func.o lib/node.o lib/stdarg.o lib/main.o lib/thread.o lib/share.so lib/libstatic_lib.a
	gcc -Llib -g -std=c11 -Wall lib/func.o lib/node.o lib/stdarg.o lib/thread.o lib/share.so -lstatic_lib -pthread lib/main.o -o cky

lib/main.o:main.c
	gcc -c main.c -o lib/main.o

lib/thread.o:src/thread.c
	gcc -c src/thread.c -o lib/thread.o


lib/static_lib.o:src/static_lib.c
	gcc -c src/static_lib.c -o lib/static_lib.o

lib/node.o:src/node.c
	gcc -c src/node.c -o lib/node.o

lib/func.o:src/func.c
	gcc -c src/func.c -o lib/func.o

lib/stdarg.o:src/stdarg.c
	gcc -c src/stdarg.c -o lib/stdarg.o

# 静态库的编译
lib/libstatic_lib.a:lib/static_lib.o
	ar rcs lib/libstatic_lib.a lib/static_lib.o

# 动态库的编译
lib/share.so:src/share.c
	gcc -shared -fPIC src/share.c -o lib/share.so

clean:
	rm lib/*.o
	rm lib/*.so
	rm lib/*.a
