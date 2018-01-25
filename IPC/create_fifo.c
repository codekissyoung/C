#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

int main( int argc, char* argv[] )
{
    mode_t mode = 0666;
    mkfifo( argv[1], mode );
    exit( 0 );
}
