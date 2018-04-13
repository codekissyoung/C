#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
void f( int );
int main( int argc, char *argv[] )
{
    int i;
    signal( SIGINT, f );
    for( i = 0; i < 5; i++ )
    {
        printf( "hello\n" );
        sleep( 1 );
    }
}
void f( int signum )
{
    printf( "out %d \n",signum );
}
