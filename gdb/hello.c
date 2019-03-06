#include <stdio.h>
int test( int a, int b )
{
	int c = a + b;
	return c;
}

int main( int argc, char *argv[] )
{
	int a = 0x1000;
	int b = 0x2000;
	int c = test( a, b );
	printf( "%d\n", c );
	printf( "Hello, World!\n");
	return 0;
}
