# makefile for cky
cky:func.o main.c
	gcc -g func.o main.c -o cky
func.o:func.c
	gcc -g -c func.c
