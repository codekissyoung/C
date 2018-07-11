cc = g++
flags = -Wall -fPIC
#flags = -g -Wall -fPIC
inc = -I../frame -I../common -I../hiredis
lib = -lpthread -ldl -L../common -lcommon -L../hiredis -lhiredis

as_srcs = $(wildcard *.cpp)
as_objs = $(patsubst %cpp,%o,$(as_srcs))
target= as.so

all: $(target)

as.so: $(as_objs)
	$(cc) $(as_objs) $(lib) -Wl,-E -shared -fPIC -o $@

%o: %cpp
	$(cc) $(flags) -c -o $@ $< $(inc)

clean:
	rm -f *.o $(target)

