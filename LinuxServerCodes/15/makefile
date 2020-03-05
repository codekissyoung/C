CC = g++
CFLAGS = -g -Wall -m64 -pthread -std=c11
SRC = $(wildcard *.cpp)
BIN = $(patsubst %.cpp,%, $(SRC))

all : $(BIN)

clean:
	$(RM) $(BIN) *.i *.o *.s cor