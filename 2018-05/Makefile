.PHONY:clean
CC=g++
CPPFLAGS=-Wall -g -pedantic
BIN=main
OBJS=main.o error.o func.o

$(BIN):$(OBJS)
	$(CC) $(CPPFLAGS) $^ -lpthread -o $@

%.o:%.c
	$(CC) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f *.o $(BIN)
