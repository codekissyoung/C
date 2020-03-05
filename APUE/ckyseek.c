#include "common.h"
#include "dbg.h"

#define BUFSIZE 1600
char buf[BUFSIZ] = {};

// ckyseek pathname offset size str
int main( int argc, char *argv[] )
{
    check( argc != 1 , "usage: ckyseek pathname offset size str");
    int fd      = open( argv[1], O_RDWR | O_CREAT, MODE_RWRWRW );
    printf("%s: \n", argv[1]);
    int read_num = read( fd, buf, sizeof(buf) );

    for( int i = 0; i < read_num; i++ )
    {
        if( i % 16 == 0 )
            printf("%.8x : ", i);
        
        printf("%.2x", buf[i]);
        switch( buf[i] )
        {
            case '\n':
                printf("(\\n)");
                break;
            case '\b':
                printf("\\b");
                break;
            default:
                if(isprint(buf[i]))
                    printf("( %c)",buf[i]);
                else
                    printf("(..)");
                break;
        }
        printf(" ");

        if( (i+1) % 16 == 0 )
            printf("\n");
    }
    printf("\n");
    close( fd );

    return 0;

error:
    return 1;
}