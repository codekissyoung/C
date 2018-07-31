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
    arr[9] = '\0';
    int len = sizeof(arr);
    int ret = 0;

    int fd = open("file.txt", O_RDONLY );
    if( fd == -1 )
        printf("open file.txt error\n");
    int i = 0;
    while( (ret = read(fd, arr, len - 1)) != 0 )
    {
        if( ret == -1 )
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
