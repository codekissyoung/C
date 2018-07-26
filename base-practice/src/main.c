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
    int fd = open("file.txt", O_RDONLY );
    if( fd == -1 )
        printf("open file.txt error\n");

    char arr[10];
    int len = sizeof(arr);
    printf("%d",len);
    int ret = 0;

    // printf("%d",ret);
    // printf("%d",errno);
    // printf("%s",strerror( errno ));
    while( (ret = read(fd, arr, len)) != 0 )
    {
        if( ret == -1 )
        {
            if( errno == EINTR )
                continue;
            perror("read error");
            break;
        }
        printf("%d",ret);
        printf("读完10个字节");
    }

    close( fd );
    return 0;
}
