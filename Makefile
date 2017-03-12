# makefile for cky
cky:func.o main.c
	gcc -L. -g -O2 -Wall func.o main.c ./share.so -lstatic_lib  -o cky

func.o:func.c

clean:
	rm *.o
