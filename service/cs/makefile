cc = g++
flags = -Wall -fPIC
#flags = -g -Wall -fPIC
inc = -I../frame -I../common -I../hiredis
lib = -lpthread -ldl -L../common -lcommon -L../hiredis -lhiredis

srcs = $(wildcard *.cpp)
objs = $(patsubst %cpp,%o,$(srcs))
target= cs.so

all: $(target)

cs.so: $(objs)
	$(cc) $(objs) $(lib) -Wl,-E -shared -fPIC -o $@

%o: %cpp
	$(cc) $(flags) -c -o $@ $< $(inc)

clean:
	rm -f *.o $(target)

