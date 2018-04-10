#include <stdio.h>
int main( int argc, char *argv[] )
{
    int c,n = 0;
    while( ( c = getchar() ) != 'Q' )
        printf("char %d is %c code %d\n", n++ , c , c );
}
