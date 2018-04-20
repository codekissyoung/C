#include "common.h"
#define BUFSIZE 100
int main( int argc, char *argv[] )
{
    int len;
    int i;
    int apipe[2];
    char buf[BUFSIZE];

    pipe(apipe);
    printf( "pipe : %d , %d", apipe[0], apipe[1] );

    while( fgets( buf, BUFSIZE, stdin ) )
    {
        len = strlen( buf );
        write( apipe[1], buf, len );

        for( i = 0; i < len; i++ )
            buf[i] = 'X';

        len = read( apipe[0], buf, BUFSIZE );

        write( 1, buf, len );
    }
}
