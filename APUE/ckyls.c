#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include "dbg.h"

int main( int argc, char *argv[] ){
    DIR             *dp;
    struct dirent   *dirp;
    if( argc == 1 ){
        argv[1] = "./";
    }else{
        check( argc == 2, "usage : ls dir-name");
    }

    dp = opendir( argv[1] );
    check( dp != NULL, "can not open dir %s", argv[1]);
    while ( ( dirp = readdir( dp ) ) != NULL ){
        printf( "%s\n", dirp->d_name );
    }
    closedir( dp );

    return 0;
    
error:
    return 1;
}