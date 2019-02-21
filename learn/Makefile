# makefile 宏
define print-dev-env
	echo -n "编译环境: ";
	uname -a;
endef

CC  		= gcc

OBJECTS 	= $(patsubst %.c,%.o,$(notdir $(wildcard src/*.c)))

DEBUG 		= -g3

CFLAGS 		= -std=gnu11 -pedantic -Wall -Werror -I include -L lib $(DEBUG)

ARFLAGS 	= rcsU

VPATH 		= lib src include
vpath 		= %.c src
vpath 		= %.h include
vpath 		= %.o %.a %.so lib

#############################  start ##############################

.PHONY = all
all: cky

cky : $(OBJECTS) -lpthread -lshare -lstatic
	$(CC) $(CFLAGS) $(OBJECTS) -pthread -lshare -lstatic -o $@
	@$(print-dev-env)

lib/libstatic.a : libstatic.a(static.o) libstatic.a(node.o)

lib/libshare.so : share.o
	$(CC) -I include -shared -fPIC $< -o $@

%.o : %.c common.h

.PHONY = clean
clean:
	rm -rf *.i
	rm -rf *.s
	rm -rf *.o
	rm -rf lib/*.so
	rm -rf lib/*.a
	rm -rf cky
