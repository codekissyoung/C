define print-dev-env
	echo -n "编译环境: ";
	uname -a;
endef

BIN = cky

CC  = gcc

OBJECTS = $(patsubst %.c,%.o,$(notdir $(wildcard src/*.c)))

DEBUG = -g3

CPPFLAGS = -std=gnu11 -pedantic -Wall -Werror $(DEBUG) -Iinclude -Llib

ARFLAGS = rcsU

SHARE_FLAGS = -Iinclude -shared -fPIC

VPATH = lib src include
vpath = %.c src
vpath = %.h include
vpath = %.o %.a %.so lib

.PHONY = all

.PHONY = clean

#############################  start ##############################

$(BIN) : $(OBJECTS) -lpthread -lshare -lstatic
	$(CC) $(CPPFLAGS) $(OBJECTS) -pthread -lshare -lstatic -o $@
	@$(print-dev-env)

lib/libstatic.a : libstatic.a(static.o) libstatic.a(node.o)

lib/libshare.so : share.o
	$(CC) $(SHARE_FLAGS) $< -o $@

%.o : %.c common.h
	$(CC) $(CPPFLAGS) -c $<

all: $(BIN)

clean:
	rm -rf *.i
	rm -rf *.s
	rm -rf *.o
	rm -rf lib/*.so
	rm -rf lib/*.a
	rm -rf cky
