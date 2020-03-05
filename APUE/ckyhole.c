#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#include "dbg.h"

char buf1[] = "abcdefghij\n";
char buf2[] = "ABCEDFGHIJ\n";

int main( int argc, char *argv[] ){

    int fd;

    check( (fd = creat( "file2.hole", (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP) )) > 0, "creat error!" );

    check( write(fd, buf1, sizeof(buf1)) == sizeof(buf1), "buf1 write error" );

    check( lseek(fd, 30, SEEK_CUR) != -1, "lseek error");

    check( write(fd, buf2, sizeof(buf2)) == sizeof(buf2), "buf2 write error" );

    return 0;
    
error:
    return 1;
}