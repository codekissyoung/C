#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "func.h"

int main( int argc, char *argv[] )
{
    char *arr = malloc(1000);
    int len = sizeof(arr);
    ssize_t ret;
    int fd  = -1;

    if( -1 == (fd = open("file.txt", O_RDONLY)) )
        printf("open file.txt error\n");

    while( 0 != len && 0 != (ret = read(fd, arr, len)) )
    {
        if( -1 == ret )
        {
            if( errno == EINTR )
                continue;
            
            perror("read error");
            break;
        }
        len -= ret;
        arr += ret;
    }
    
    close( fd );
    return 0;
}
