#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main( int argc, char *argv[] )
{
    int fd[2];
    char str[256];
    char* s_str = "create the pipe successfully !\n";
    if( (pipe(fd)) < 0 )
    {
        perror( "pipe error\n" );
        exit( 1 );
    }

    printf("fd[0] : %d , fd[1] : %d \n", fd[0], fd[1] );

    write( fd[1], s_str, (size_t)strlen(s_str) );

    read( fd[0], str, sizeof(str) );

    printf( "%s", str );

    close( fd[0] );
    close( fd[1] );
    return 0;
}
