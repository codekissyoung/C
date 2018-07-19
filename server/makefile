.PHONY:clean
CC=g++
CPPFLAGS=-Wall -g -pedantic -std=c++11

BIN=epoll-server
OBJS=main.o

$(BIN):$(OBJS)
	$(CC) $(CPPFLAGS) $^ -lpthread -o $@

%.o:%.c
	$(CC) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f *.o $(BIN)
