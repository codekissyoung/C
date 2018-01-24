#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>

#define PIPE_BUF 255
int main( int argc, char* argv[] )
{
    int fd[2];
    pipe( fd );
    pid_t pid = fork();
    if( pid > 0 )
    {
        close( fd[0] );
        write( fd[1] , "hello my son \n ",14);
        exit(0);
    }
    else
    {
        close( fd[1] );
        char buf[ PIPE_BUF ];
        int len = read( fd[0], buf, PIPE_BUF );

        write( STDOUT_FILENO, buf, len );
        exit(0);
    }

}

