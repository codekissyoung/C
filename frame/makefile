CC = gcc
flags = -g -Wall -fPIC
lib = -lpthread -ldl

srcs = $(wildcard *.c)
server_srcs = buffer.c conf.c event.c io.c log.c thread.c server.c
server_objs = $(patsubst %c,%o,$(server_srcs))

test_mod_srcs = mod_test.c
test_mod_objs = $(patsubst %c,%o,$(test_mod_srcs))

client_srcs = client.c
client_objs = $(patsubst %c,%o,$(client_srcs))

target= server test_mod.so client

all: $(target)

server: $(server_objs)
	$(cc) $(server_objs) $(lib) -Wl,-E -o $@

test_mod.so: $(test_mod_objs)
	$(cc) $(test_mod_objs) $(lib) -Wl,-E -shared -fPIC -o $@

client: $(client_objs)
	$(cc) $(client_objs) $(lib) -Wl,-E -o $@

%o: %c
	$(cc) $(flags) -c -o $@ $<

clean:
	rm -f *.o $(target)
