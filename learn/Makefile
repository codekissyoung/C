define print-dev-env
	echo -n "编译环境: ";
	uname -a;
endef

BIN = cky

CC  = gcc

OBJECTS = $(patsubst %.c,%.o,$(notdir $(wildcard src/*.c)))

DEBUG = -g3

CPPFLAGS = -std=gnu11 -Iinclude -Llib -pedantic -Wall $(DEBUG)

ARFLAGS = rcsvU

SHARE_FLAGS = -Iinclude -shared -fPIC

VPATH = lib src include
vpath = %.c src
vpath = %.h include
vpath = %.o %.a %.so lib

$(BIN) : $(OBJECTS) -lpthread libshare.so libstatic.a
	$(CC) $(CPPFLAGS) $(OBJECTS) -pthread -lstatic -lshare -o $@
	@$(print-dev-env)

lib/libstatic.a : libstatic.a(static.o) libstatic.a(node.o)

lib/libshare.so : share.o
	$(CC) $(SHARE_FLAGS) $< -o $@

%.o : %.c common.h
	$(CC) $(CPPFLAGS) -c $< -o $@

.PHONY = all
all: $(BIN)

.PHONY = clean
clean:
	-rm -rf *.i
	-rm -rf *.s
	-rm -rf *.o
	-rm -rf lib/*.so
	-rm -rf lib/*.a
	-rm -rf cky
