BIN = cky
CC  = gcc
OBJECTS = func.o node.o stdarg.o main.o thread.o global_variables.o
DEBUG = -g
CPPFLAGS = -std=gnu11 -pedantic -Wall -Iinclude -Llib # -Werror
VPATH = lib src include
vpath = %.c src
vpath = %.h include
vpath = %.o %.a %.so lib

$(BIN) : $(OBJECTS) -lpthread libshare.so libstatic.a
	$(CC) $(DEBUG) $(CPPFLAGS) -pthread -lstatic -lshare $(OBJECTS) -o $@

libstatic.a : static.o
	ar rcs lib/$@ $<

libshare.so : share.c
	$(CC) -I include -shared -fPIC $< -o lib/$@

.INTERMEDIATE = %.o
%.o : %.c
	$(COMPILE.c) $< -o $@

clean:
	-rm -rf *.o
	-rm -rf lib/*.so
	-rm -rf lib/*.a
	-rm -rf cky
