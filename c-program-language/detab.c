#include <stdio.h>

#define TABINC 8

int main( int argc, char *argv[] )
{
    int c, nb, pos;
    nb  = 0;
    pos = 0;
    while( ( c = getchar() ) != EOF )
    {
        switch( c )
        {
            case '\t':
                nb = TABINC - pos % TABINC;
                while( nb > 0 )
                {
                    putchar(' ');
                    pos++;
                    nb--;
                }
                break;

            case '\n':
                putchar( c );
                pos = 0;
                break;

            default:
                putchar( c );
                pos++;
                break;
        }
    }
}
