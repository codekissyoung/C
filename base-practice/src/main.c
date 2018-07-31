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
    char arr[10];
    arr[9]  = '\0';
    int len = sizeof(arr);
    int ret = 0;
    int i   = 0;
    int fd  = -1;

    if( -1 == (fd = open("file.txt", O_RDONLY)) )
        printf("open file.txt error\n");

    while( 0 != (ret = read(fd, arr, len - 1)) )
    {
        if( -1 == ret )
        {
            if( errno == EINTR )
                continue;
            perror("read error");
            break;
        }
        printf("%d : %s\n",i,arr);
        i++;
    }
    close( fd );
    return 0;
}
