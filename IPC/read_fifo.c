#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>

#define BUFES 256

int main( int argc, char* argv[] )
{
    int fd;
    int len = 0;
    char buf[BUFES];
    mode_t mode = 0666;

    if( ( fd = open( "fifo1", O_RDONLY ) ) < 0 )
    {
        perror("open error\n");
        exit(1);
    }

    len = read( fd, buf, BUFES );

    printf( "%d",len);

    while( ( len = read( fd, buf, BUFES ) ) > 0 )
    {
        printf("read info from fifo1 : %s\n", buf );
    }
    close( fd );
    return 0;
}
