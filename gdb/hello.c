#include <stdio.h>

void swap( int *p1, int *p2 );
int test( int a, int b );
long double factorial( register unsigned int n );

int main( int argc, char *argv[] )
{
	int a = 0x1000;
	int b = 0x2000;

    int *iPtr = &a;
    ++*iPtr;
    puts( "This is the statement following ++ *iPtr." );
    printf( "a = %d; b = %d.\n", a, b );

    char msg[100] = "hello world!\n";
    char *cPtr = msg + 6;

    int f = factorial( 5 );

    printf( "old value : a = %d; b = %d.\n", a, b );

    swap( &a, &b );

    printf( "new value : a = %d; b = %d.\n", a, b );
    
	int c = test( a, b );
    printf( "%d\n", c );
	printf( "Hello, World!\n");

	return 0;
}

void swap( int *p1, int *p2 )
{
    int *p = p1;
    p1 = p2;
    p2 = p;
}


int test( int a, int b )
{
	int c = a + b;
	return c;
}

long double factorial( register unsigned int n )
{
    if ( n <= 1 )
        return 1.0L;
    else
        return n * factorial( n - 1 );
}
