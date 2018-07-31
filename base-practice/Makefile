# 定义变量
BIN=main
CC=gcc
SRC_DIR=src
INCLUDE_DIR=/home/cky/workspace/C/base-practice/include

CPPFLAGS=-Wall -g -pedantic
LINK_LIB=-lpthread
SEARCH_DIR=-I$(INCLUDE_DIR)

OBJS=main.o func.o

.PHONY: all
all:$(BIN)

VPATH=src obj include
vpath %.c src
vpath %.h include

# 书写构建规则
$(BIN):$(OBJS)
	$(CC) $(CPPFLAGS) $^ $(SEARCH_DIR) $(LINK_LIB) -o $@

# 隐含规则
%.o:%.c
	$(CC) $(CPPFLAGS) -c $< $(SEARCH_DIR) $(LINK_LIB) -o $@

.PHONY:clean
clean:
	rm -f *.o $(BIN)
