helloworld:helloworld.o
	gcc $^ -o $@
helloworld.s:helloworld.i
	gcc -S $^ -o $@
helloworld.i:hello_world.c
	gcc -E $^ -o $@
helloworld.o:helloworld.s
	gcc -c $^ -o $@

clean:
	rm *.i
	rm *.s
	rm *.o
	rm helloworld
