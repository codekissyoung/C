#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 8

int main( int argc, char *argv[] )
{
    char buff[ BUF_SIZE ];

    // 输入的文件
    int input_fd = open( argv[1], O_RDONLY );

    while( read( input_fd, buff, BUF_SIZE ) > 0 )
    {
        printf( "%s", buff );
    }

    // int output_fd = open( argv[2], O_WRONLY, filePerms );

    close( input_fd );
}
