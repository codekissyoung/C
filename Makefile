# makefile for cky
cky:func.o main.c
	gcc func.o main.c -o cky
func.o:func.c
	gcc -c func.c
