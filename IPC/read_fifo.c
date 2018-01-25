#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>

#define BUFES PIPE_BUF

int main( int argc, char* argv[] )
{
    int fd;
    int len = 0;
    char buf[BUFES];

    if( ( fd = open( "/home/cky/workspace/C/IPC/fifo1", O_RDONLY ) ) < 0 )
    {
        perror("open error\n");
        exit(1);
    }

    while( ( len = read( fd, buf, BUFES ) ) > 0 )
    {
        printf("read info from fifo1 : %s\n", buf );
    }
    printf("hehe");
    close( fd );
    return 0;
}
