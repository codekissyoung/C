#include <stdio.h>
int main( int argc, char* argv[] )
{
    int (*p)[10];

    int a[1][10] = {0,1,2,3,4,5,6,7,8,9};

    int *p_int;

    p = a;
    p_int = ( int* )p;

    for( int i = 0; i < sizeof(*p) / sizeof(int); i++ )
    {
        printf( "%d ",*(p_int+i) );
    }
    return 0;
}
