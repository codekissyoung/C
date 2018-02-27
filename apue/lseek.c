#include "apue.h"
int main( int argc, char* argv[] )
{
    if( lseek(STDIN_FILENO,0,SEEK_CUR) == -1 )
        printf("can not seek");
    else
        printf("ok");
    printf("\n");
    exit( 0 );
}
