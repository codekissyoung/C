#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main( int argc, char *argv[] )
{
    int *array = (int*)malloc( 8 * sizeof(int) );
    array[0] = 190;
    array[4] = 90;
    array[7] = 76;
    return 0;
}
