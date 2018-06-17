#include <stdio.h>
#include <ctype.h>

int main( int argc ,char *argv[] )
{
    int c;
    while(  ( c = getchar() ) != EOF )
    {
        if( c == 'z' )
            c = 'a';
        else if( islower(c) )
            c++;
        putchar(c);
    }
}
