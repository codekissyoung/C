FLAGS=-p -Wall -Wcast-align -std=gnu11  -pedantic -I. -L. -pthread
LIB=
DEBUG=-g3

all:main server client epoll-server

# 可执行程序
main:main.o question.o log.o libsecurity.a libsort.so
	gcc $(DEBUG) $(FLAGS) $(LIB) $^ -o $@

server:server.o
	gcc $(DEBUG) $(FLAGS) $(LIB) $^ -o $@

client:client.o
	gcc $(DEBUG) $(FLAGS) $(LIB) $^ -o $@

epoll-server:epoll-server.o
	gcc $(DEBUG) $(FLAGS) $(LIB) $^ -o $@

main.o:main.c main.h common.h
	gcc $(DEBUG) $(FLAGS) -c $< -o $@

# 库
libsecurity.a:encrypt.o checksum.o
	ar -rcs libsecurity.a encrypt.o checksum.o

libsort.so:sort.o
	gcc -shared -fPIC $^ -o $@

# 临时文件
encrypt.o:encrypt.c encrypt.h
	gcc $(DEBUG) $(FLAGS) -c $< -o $@

checksum.o:checksum.c checksum.h
	gcc $(DEBUG) $(FLAGS) -c $< -o $@

question.o:question.c question.h
	gcc $(DEBUG) $(FLAGS) -c $< -o $@

sort.o:sort.c sort.h
	gcc $(DEBUG) $(FLAGS) -fPIC -c $< -o $@

log.o:log.c log.h
	gcc $(DEBUG) $(FLAGS) -c $< -o $@

server.o:server.c server.h
	gcc $(DEBUG) $(FLAGS) -c $< -o $@

client.o:client.c client.h
	gcc $(DEBUG) $(FLAGS) -c $< -o $@

epoll-server.o:epoll-server.c epoll-server.h
	gcc $(DEBUG) $(FLAGS) -c $< -o $@

# 清理操作
.PHONY:clean
clean:
	rm *.o
	rm *.a
	rm *.so
	rm main
	rm client
	rm server
	rm epoll-server