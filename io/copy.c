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

    // 输出的文件
    int output_fd = open( argv[2],
                          O_CREAT | O_WRONLY | O_TRUNC,
                          S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );

    ssize_t read_num;
    while( (read_num = read( input_fd, buff, BUF_SIZE )) > 0 )
    {
        if( write( output_fd, buff, read_num ) != read_num )
            printf("can not write whole buffer\n");
    }

    close( input_fd );
    return 0;

}
