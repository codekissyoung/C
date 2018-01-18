#include <stdio.h>
int main ( int argc, char* argv[] )
{
    int int_arr[] = { 1, 2, 3, 4, 5 };
    int* p;
    int** pp;

    p = int_arr;

    pp = &p;

    for( int i; i < 5; i++ )
    {
        printf( " ** pp[%d] : %d", i, (* pp)[i] );
    }
    return 0;
}
