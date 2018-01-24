#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>

#define BUFSZ 256

int main ( int argc, char* argv[] )
{
    int fd[2];
    pipe( fd );

    pid_t pid = fork();
    if( pid ==  0 )
    {
        close( fd[0] );
        write( fd[1], "hello brother!", 14 );
        exit( 0 );
    }

    pid_t pid2 = fork();
    if( pid2 == 0 )
    {
        close( fd[1] );
        char buf[BUFSZ];
        int len = read( fd[0], buf, BUFSZ );
        write( STDOUT_FILENO, buf, len );
        exit( 0 );
    }

    close( fd[0] );
    close( fd[1] );
    return 0;
}
