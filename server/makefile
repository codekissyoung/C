CC = gcc
CPPFLAGS = -Wall -g -m64 -pedantic -std=c11 -pthread -DDEBUG

SRC = $(wildcard *.c)
BIN = $(patsubst %.c,%, $(SRC))

all : $(BIN)

.PHONY:clean
clean:
	$(RM) $(BIN) *.i *.o *.s core
