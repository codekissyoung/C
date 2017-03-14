# makefile for cky
cky:func.o share.so libstatic_lib.a main.c
	gcc -L. -g -O2 -Wall func.o main.c ./share.so -lstatic_lib  -o cky

# 静态库的编译
libstatic_lib.a:static_lib.o
	ar rcs libstatic_lib.a static_lib.o

static_lib.o:static_lib.c

# 动态库的编译
share.so:share.c
	gcc -shared -fPIC share.c -o share.so

func.o:func.c

clean:
	rm *.o
	rm *.so
	rm *.a
