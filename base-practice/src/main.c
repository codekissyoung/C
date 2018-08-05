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

#define TIMEOUT 3
#define BUF_LEN 1024

int main( int argc, char *argv[] )
{
    pid_t pid;
    pid = fork();
    if( pid < 0 )
        perror("fork");
    if( pid == 0 )
    {
        int fd  = open("file.txt",O_RDONLY);
        int fd2 = open("file.txt",O_APPEND | O_WRONLY);
        char buffer[] = "abcdefg";
        int ret2 = write(fd2,buffer,sizeof(buffer) - 1);
        if( ret2 != -1 )
            printf("success! ret2 : %d \n", ret2);

        char arr[100];
        int ret = read(fd,arr,100);
        if(ret != -1)
        {
            printf("size : %ld\n",sizeof(arr));
            arr[ret] = '\0';
            for(int i = 0; i < 100; i++ )
            {
                if( arr[i] == '\0' )
                {
                    printf("arr[%d] : \\0 \n",i);
                    break;
                }
                else
                    printf("arr[%d] : %c \n",i,arr[i]);
            }
            printf("arr: %s\n",arr);
        }
        close(fd);
        close(fd2);
        printf("子进程 pid : %d, ppid: %d, sid: %d \n",getpid(),getppid(),getsid(getpid()));
        exit(0);
    }
    sleep(1);
    printf("父进程 pid : %d, ppid: %d, sid: %d\n",getpid(),getppid(),getsid(getpid()));

    struct timeval tv;
    fd_set readfds;
    int ret;

    FD_ZERO(&readfds);

    FD_SET(STDIN_FILENO,&readfds);

    tv.tv_sec  = TIMEOUT;
    tv.tv_usec = 0;

    ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);

    if( ret == -1 )
    {
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
