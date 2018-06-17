.PHONY:clean
CC=gcc
CPPFLAGS=-Wall -g -std=c11 -pedantic
BIN=client
OBJS=client.o

$(BIN):$(OBJS)
	$(CC) $(CPPFLAGS) $^ -o $@

%.o:%.c
	$(CC) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f *.o $(BIN)
