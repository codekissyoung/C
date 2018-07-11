build = release
cc = g++

flags = -O2 -std=c++11 -Wall -fPIC

inc = -I../ -I../frame -I../common -I../hiredis -I./bson
lib = -lpthread -ldl -L../common -lcommon -L../hiredis -lhiredis -lmongoc-1.0

srcs = $(wildcard *.cpp)
objs = $(patsubst %cpp,%o,$(srcs))
target= rs.so

all: $(target)

rs.so: $(objs)
	$(cc) $(objs) $(lib) -Wl,-E -shared -fPIC -o $@

%o: %cpp
	$(cc) $(flags) -c -o $@ $< $(inc)

clean:
	rm -f *.o $(target)

