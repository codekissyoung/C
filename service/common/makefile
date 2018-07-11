cc = g++
#flags = -g -Wall -fPIC
flags = -Wall -fPIC
inc = -I../frame -I.
lib = -lpthread -ldl

srcs = $(wildcard *.cpp)
objs = $(patsubst %cpp,%o,$(srcs))
target = libcommon.a

all: $(target)

libcommon.a: $(objs)
	ar -r $(target) $(objs)

%o: %cpp
	$(cc) $(flags) -c -o $@ $< $(inc)

clean:
	rm -f *.o $(target)

