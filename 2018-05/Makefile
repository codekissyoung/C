.PHONY:clean
CC=gcc
CPPFLAGS=-Wall -g -std=c11 -pedantic
BIN=main
OBJS=main.o error.o func.o

$(BIN):$(OBJS)
	$(CC) $(CPPFLAGS) $^ -o $@

%.o:%.c
	$(CC) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f *.o $(BIN)
