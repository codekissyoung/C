#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include "dbg.h"

#define BUFFSIZE 4096

int main( int argc, char *argv[] ){
    int     n;
    char    buf[BUFFSIZE];

    while( ( n = read(STDIN_FILENO, buf, BUFFSIZE) ) > 0 ){
        if( write(STDOUT_FILENO, buf, n) != n)
            log_err("write error");       
    }
    return 0;
}