#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void my_print( char *s )
{
    printf( "The string is : %s \n", s );
}

void my_print2( char *s )
{
    int size = strlen( s ) - 1; // length with no '\0'
    char *s2 = (char*)malloc( size + 1 );

    for(int i = 0; i < size + 1; i++ )
    {
        s2[ size - i ] = s[i];
    }
    s2[ size + 1 ] = '\0';
    printf( "The reverse of string is : %s \n", s2 );
}

int main( int argc, char *argv[] )
{
    char my_string[] = "Hello there";
    my_print( my_string );
    my_print2( my_string );
    return 0;
}
