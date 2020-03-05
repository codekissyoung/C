CC = gcc
CFLAGS = -g -Wall -m64 -pthread -std=c11
SRC = $(wildcard *.c)
BIN = $(patsubst %.c,%, $(SRC))

all : $(BIN)

clean:
	$(RM) $(BIN) *.i *.o *.s core
