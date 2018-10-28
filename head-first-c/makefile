FLAGS=-p -Wall -Wcast-align -std=gnu11  -pedantic
DEBUG=-g3

main:main.o encrypt.o question.o sort.o
	gcc $(DEBUG) $(FLAGS) $^ -o $@

main.o:main.c main.h common.h
	gcc $(DEBUG) $(FLAGS) -c $< -o $@

encrypt.o:encrypt.c encrypt.h
	gcc $(DEBUG) $(FLAGS) -c $< -o $@

question.o:question.c question.h
	gcc $(DEBUG) $(FLAGS) -c $< -o $@

sort.o:sort.c sort.h
	gcc $(DEBUG) $(FLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm main
	rm *.o
