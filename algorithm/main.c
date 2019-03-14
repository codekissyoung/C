#include <stdio.h>

void f( int x )
{
    printf("%d\n",x);
}

const int *findValue( const int *begin, const int *end, int value )
{
    while( begin != end && *begin != value )
        ++ begin;
    return begin;
}

int main( int argc, char *argv[] )
{
    int data[] = {2, 3, 4, 6, 9, 10, 34, 44};
    
    const int *ip = findValue( data, &data[ sizeof(data) / sizeof(int) - 1 ], 9 );

    printf( "%d \n", *ip );

    return 0;
}
