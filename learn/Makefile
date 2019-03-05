BIN         = cky

CC  		= gcc

OBJECTS 	= $(patsubst %.c,%.o,$(notdir $(wildcard src/*.c)))

DEBUG 		= -g3

CFLAGS 		= -std=gnu11 -pedantic -Wall -Werror -Iinclude $(DEBUG)

ARFLAGS 	= rcsU

VPATH 		= lib src include
vpath 		= %.c src
vpath 		= %.h include
vpath 		= %.o %.a %.so lib

#############################  start ##############################

.PHONY = all
all : $(BIN)

$(BIN) : $(OBJECTS) -lpthread libstatic.a libshare.so
	$(CC) -o $@ $(CFLAGS) $(OBJECTS) -pthread -L. -lshare -lstatic

libstatic.a : libstatic.a(static.o) libstatic.a(node.o)

libshare.so : share.o
	$(CC) -o $@ -Iinclude -shared -fPIC $<

%.o : %.c common.h

.PHONY = clean
clean:
	$(RM) *.i
	$(RM) *.s
	$(RM) *.o
	$(RM) *.so
	$(RM) *.a
	$(RM) lib/*.so
	$(RM) lib/*.a
	$(RM) $(BIN)
