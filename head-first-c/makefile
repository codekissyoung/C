FLAGS=-p -Wall -Wcast-align -std=gnu11  -pedantic -I. -L. -pthread
LIB=
DEBUG=-g3

all:main server client

main:main.o question.o log.o libsecurity.a libsort.so
	gcc $(DEBUG) $(FLAGS) $(LIB) $^ -o $@

server:server.o
	gcc $(DEBUG) $(FLAGS) $(LIB) $^ -o $@

client:client.o
	gcc $(DEBUG) $(FLAGS) $(LIB) $^ -o $@

libsecurity.a:encrypt.o checksum.o
	ar -rcs libsecurity.a encrypt.o checksum.o

libsort.so:sort.o
	gcc -shared $^ -o $@

main.o:main.c main.h common.h
	gcc $(DEBUG) $(FLAGS) -c $< -o $@

encrypt.o:encrypt.c encrypt.h
	gcc $(DEBUG) $(FLAGS) -c $< -o $@

checksum.o:checksum.c checksum.h
	gcc $(DEBUG) $(FLAGS) -c $< -o $@

question.o:question.c question.h
	gcc $(DEBUG) $(FLAGS) -c $< -o $@

sort.o:sort.c sort.h
	gcc $(DEBUG) $(FLAGS) -c $< -o $@

log.o:log.c log.h
	gcc $(DEBUG) $(FLAGS) -c $< -o $@

server.o:server.c server.h
	gcc $(DEBUG) $(FLAGS) -c $< -o $@

client.o:client.c client.h
	gcc $(DEBUG) $(FLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm *.o
	rm *.a
	rm *.so
	rm main
	rm client
	rm server