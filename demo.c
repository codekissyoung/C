#include <stdio.h>
int main( int argc, char *argv[] )
{
    int arr[] = { 2, 5, 1, 4, 9, 1, 3, 8, 10, 2, 4, 9};
    int max = 0;
    int len = sizeof(arr) / sizeof(int);
    for ( int i = 0; i < len; i++ )
    {
        if( arr[i] > max )
            max = arr[i];
    }

    for( int i = 0; i < max; i++ )
    {
        for( int j = 0; j < len; j++ )
        {
            if( max - arr[j] - i > 0 )
                printf(" ");
            else
                printf("*");
        }
        printf("\n");
    }
    return 0;
}
