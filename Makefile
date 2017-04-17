# makefile for cky
cky:func.o node.o main.o share.so libstatic_lib.a
	gcc -L. -g -std=c11 -O2 -Wall func.o node.o main.c ./share.so -lstatic_lib  -o cky

main.o:main.c
static_lib.o:static_lib.c
node.o:node.c
func.o:func.c

# 静态库的编译
libstatic_lib.a:static_lib.o
	ar rcs libstatic_lib.a static_lib.o

# 动态库的编译
share.so:share.c
	gcc -shared -fPIC share.c -o share.so

clean:
	rm *.o
	rm *.so
	rm *.a
