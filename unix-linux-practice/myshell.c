#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define MAXARGS 20
#define ARGLEN  100

char *makestring( char *buf );
int execute( char* arglist[] );

int main( int argc, char *argv[] )
{
    char *arglist[ MAXARGS + 1 ];
    int numargs = 0;
    char argbuf[ ARGLEN ];

    while( numargs < MAXARGS )
    {
       printf( "Arg[%d] : \n", numargs ); 
       fgets( argbuf, ARGLEN, stdin );
       
       if( strcmp( argbuf, "\n" ) )
       {
           arglist[ numargs++ ] = makestring( argbuf );
       }
       else
       {
           if( numargs > 0 )
           {
               arglist[numargs] = NULL;
               execute( arglist ); 
               numargs = 0;
           }
       }
    }
    return 0;
}

char *makestring( char *buf )
{
    char *cp;
    buf[ strlen( buf ) - 1 ] = '\0';
    cp = malloc( strlen( buf ) + 1 );
    strcpy( cp, buf );
    return cp;
}

int execute( char* arglist[] )
{
    execvp( arglist[0], arglist );
    perror("execvp failed\n");
    exit( 1 );
}
