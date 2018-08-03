#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "func.h"

#define TIMEOUT 1
#define BUF_LEN 1024

int main( int argc, char *argv[] )
{
    struct timeval tv;
    fd_set readfds;
    int ret;


    pid_t pid;
    pid = fork();
    if( pid < 0 )
        perror("fork");
    if( pid == 0 )
    {
        printf("子进程 pid : %d, ppid: %d, sid: %d \n",getpid(),getppid(),getsid(getpid()));
        exit(0);
    }
    sleep(1);
    printf("父进程 pid : %d, ppid: %d, sid: %d\n",getpid(),getppid(),getsid(getpid()));

    // ret = execl("/usr/bin/vim","vim","src/main.c",NULL);
    // if(ret == -1)
    //     perror("execl");

    FD_ZERO(&readfds);

    FD_SET(STDIN_FILENO,&readfds);

    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;

    ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);

    if(ret == -1){
        perror("select");
        return 1;
    }
    else if( !ret )
    {
        printf("%d seconds elapsed\n",TIMEOUT);
        return 0;
    }
    if(FD_ISSET(STDIN_FILENO, &readfds))
    {
        char buf[BUF_LEN+1];
        int len;
        len = read(STDIN_FILENO,buf,BUF_LEN);
        if(len == -1)
        {
            perror("read");
            return 1;
        }
        if(len)
        {
            buf[len] = '\0';
            printf("read: %s\n",buf);
        }
        return 0;
    }
    fprintf(stderr,"This should not happen\n");
    return 1;
}
