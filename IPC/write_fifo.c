#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <string.h>

#define BUFES 256

int main( int argc, char* argv[] )
{
    int fd;
    int n,i;
    char buf[BUFES];
    time_t tp;

    printf("I am %d \n", getpid() );

    if( ( fd == open( "/home/cky/workspace/C/IPC/fifo1",O_WRONLY) ) < 0 )
    {
        perror("open");
        exit(1);
    }
    for( i = 0; i < 10; i++ )
    {
        time( &tp );
        n = sprintf( buf, "write info : %d sends %s", getpid(), ctime(&tp) );

        if( write( fd, buf, n + 1 ) < 0 )
        {
            perror("write error\n");
            close( fd );
            exit(1);
        }
        sleep( 3 );
    }
    close( fd );
    exit(0);
}
