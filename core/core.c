#include <stdio.h>
#include <stdlib.h>

void test()
{
	char *s = "abc";
	*s = 'x';
}

int main( int argc, char *argv[] )
{
	test();
	return EXIT_SUCCESS;
}
