#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#define INPUTLEN 100

void inthandler( int );
void quithandler( int );

int main( int argc, char *argv[] )
{
    char input[INPUTLEN];
    int nchars;

    signal( SIGINT, inthandler );
    signal( SIGQUIT, quithandler );

    do{
        printf("\nType a message\n");
        nchars = read( 0, input, INPUTLEN - 1 );
        input[nchars] = '\0';
        printf( "you typed : %s", input );
    }while( strncmp( input, "quit", 4 ) != 0 );
    
}

void inthandler( int s )
{
    printf( "inthandler %d .... \n", s );
    sleep( 2 );
    printf( "leave int handler\n" );
}

void quithandler( int s )
{
    printf( "quithandler %d .... \n", s );
    sleep( 3 );
    printf( "leave quit handler\n" );
}
