BIN = cky
CC  = gcc
OBJECTS = func.o stdarg.o thread.o global_variables.o main.o
DEBUG = -g
CPPFLAGS = -std=gnu11 -Iinclude -Llib # -pedantic -Wall -Werror -save-temps
ARFLAGS = rcsvU
SHARE_FLAGS = -Iinclude -shared -fPIC 
VPATH = lib src include
vpath = %.c src
vpath = %.h include
vpath = %.o %.a %.so lib

define print-dev-env
echo "compiled at :";
uname -a
lsb_release -a
echo ${MAKE_VERSION}
endef

$(BIN) : $(OBJECTS) -lpthread libshare.so libstatic.a
	$(CC) $(DEBUG) $(CPPFLAGS) $(OBJECTS) -pthread -lstatic -lshare -o $@
	@$(print-dev-env)

lib/libstatic.a : libstatic.a(static.o) libstatic.a(node.o)

lib/libshare.so : share.c
	$(CC) $(SHARE_FLAGS) $< -o $@

%.o : %.c common.h
	$(COMPILE.c) $< -o $@

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

