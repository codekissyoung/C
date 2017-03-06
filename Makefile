# makefile for cky
cky:func.o main.c
	gcc -g -O2 -Wall func.o main.c -o cky
func.o:func.c
	gcc -g -O2 -c -Wall func.c -o func.o
