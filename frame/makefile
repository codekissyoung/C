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
	$(CC) $(server_objs) $(lib) -Wl,-E -o $@
test_mod.so: $(test_mod_objs)
	$(CC) $(test_mod_objs) $(lib) -Wl,-E -shared -fPIC -o $@
client: $(client_objs)
	$(CC) $(client_objs) $(lib) -Wl,-E -o $@
%o: %c
	$(CC) $(flags) -c -o $@ $<
clean:
	rm -f *.o $(target)
