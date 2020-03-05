CC = g++
CPPFLAGS = -g -Wall -m64 -pthread -std=c++11 -pthread
SRC = $(wildcard *.cpp)
BIN = $(patsubst %.cpp,%, $(SRC))

all : $(BIN)

clean:
	$(RM) $(BIN) *.i *.o *.s cor
