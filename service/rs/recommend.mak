build = release
cc = g++

flags = -O2 -std=c++11 -Wall -fPIC

inc = -I../ -I../frame -I../common -I./bson
lib = -lpthread -ldl -lmongoc-1.0 -lbson-1.0

srcs = main.cpp recommend.cpp user_graph.cpp user_node.cpp mongo_handle.cpp ../common/server_manager.cpp ../frame/log.c ../frame/conf.c
objs = $(patsubst %cpp,%o,$(srcs))
target= recommend

all: $(target)

recommend: $(objs)
	$(cc) $(objs) $(lib) -Wl,-E -o $@

%o: %cpp
	$(cc) $(flags) -c -o $@ $< $(inc)

clean:
	rm -f *.o $(target)

