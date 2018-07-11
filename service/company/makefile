build = release
cc = g++

ifeq ($(build), debug)
flags = -g -D_DEBUG -Wall -fPIC
else
flags = -Wall -fPIC
endif

inc = -I../frame -I../common -I../hiredis
lib = -lpthread -ldl -L../common -lcommon -L../hiredis -lhiredis

srcs = $(wildcard *.cpp)
objs = $(patsubst %cpp,%o,$(srcs))
target= company.so

all: $(target)

company.so: $(objs)
	$(cc) $(objs) $(lib) -Wl,-E -shared -fPIC -o $@

%o: %cpp
	$(cc) $(flags) -c -o $@ $< $(inc)

clean:
	rm -f *.o $(target)

