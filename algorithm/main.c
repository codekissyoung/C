#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#pragma pack(8)

void swap( int *a, int *b )
{/*{{{*/
    int c = *a;
    *a = *b;
    *b = c;
}/*}}}*/

void bubble_sort( int *begin, int *end )
{/*{{{*/

    bool sorted = false;

    while( ! sorted )
    {
        sorted = true;

        for( int *i = begin; i < end; i++ )
        {
            if( *i > *( i + 1 ) )
            {
                swap( i, i + 1 );
                sorted = false;
            }
        }

        end--;
    }
}/*}}}*/

// 斐波那契数列
int fib( int n )
{/*{{{*/
    if( n == 1 )
        return 1;

    if( n == 2 )
        return 1;

    int fn;         // n 项
    int fn_1 = 1;   // n - 1 项
    int fn_2 = 1;   // n - 2 项

    for( int i = 2; i < n; i++ )
    {
        fn = fn_1 + fn_2;
        fn_2 = fn_1;
        fn_1 = fn;
    }
    
    return fn;
}/*}}}*/

void print_arr( int *begin, int *end )
{/*{{{*/
    for( int *i = begin; i < end; i++ )
        printf("%d\t", *i);
}/*}}}*/

int main( int argc, char *argv[] )
{

    printf( "fib(5): %d\n", fib(5) );

    int arr[] = {8, 4, 6, 5, 8, 3, 1, 9, 4, 2, 1};
    size_t length = sizeof(arr) / sizeof(int);
    
    bubble_sort( &arr[0], &arr[length - 1] );

    print_arr( &arr[0], &arr[length - 1] );

    printf("\n");
    return 0;
}
