CC = g++
CPPFLAGS = -g -Wall -m64 -levent -std=c++11
SRC = $(wildcard *.cpp)
BIN = $(patsubst %.cpp,%, $(SRC))

all : $(BIN)

clean:
	$(RM) $(BIN) *.i *.o *.s cor
