#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <utmp.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "func.h"

#define TIMEOUT 3
#define BUF_LEN 1024
#define USER_PROCESS 7
#define BUFFER_SIZE 10
#ifndef UTMP_FILE
    #define UTMP_FILE ""
#endif

int main( int argc, char *argv[] )
{
    pid_t pid;
    int ret;
    pid = fork();

    // 子进程
    if( pid == 0 )
    {
        printf("子进程 pid : %d, ppid: %d, sid: %d \n",getpid(),getppid(),getsid(getpid()));
        exit(0);
    }
    
    // 父进程
    usleep(100);
    printf("父进程 pid : %d, ppid: %d, sid: %d\n",getpid(),getppid(),getsid(getpid()));

    struct utmp current_record;
    int utmpfd;
    int reclen = sizeof(current_record);
    ret = utmpfd = open( UTMP_FILE, O_RDONLY );
    while( read(utmpfd, &current_record, reclen) == reclen )
    {
        if(current_record.ut_type != USER_PROCESS )
            continue;
        printf("%s\t %s\t %12.12s\n",current_record.ut_name,
                current_record.ut_line,ctime((time_t*)(&current_record.ut_time))+4);

    }
    close(utmpfd);

    // 拷贝复制
    int in_fd  = open("file.txt",O_RDONLY);
    int out_fd = open("file_copy.txt",O_WRONLY | O_CREAT, 0644);
    char buf[BUFFER_SIZE];
    int n_chars;
    while( ( n_chars = read(in_fd,buf,BUFFER_SIZE) ) > 0 )
    {
        if( write(out_fd,buf,n_chars) != n_chars )
            perror("copy error\n");
    }
    close(in_fd);
    close(out_fd);

    struct timeval tv;
    fd_set readfds;

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
