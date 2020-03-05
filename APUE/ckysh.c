#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "dbg.h"

#define MAXLINE 512

void sig_int( int sig_num ){
    printf("interrupt\n");
    fflush(stdout);
}

int main( int argc, char *argv[] ){
    char        buf[MAXLINE];
    pid_t       pid;
    int         status;
    int         cmd_lenth;

    check( signal( SIGINT, sig_int ) != SIG_ERR, "signal error");

    printf("%% ");
    while ( fgets(buf, MAXLINE, stdin) != NULL )
    {
        cmd_lenth = strlen( buf );
        if( buf[cmd_lenth - 1] == '\n' ){
            buf[cmd_lenth - 1] = '\0';
        }

        pid = fork();

        check( pid >= 0, "fork error" );
        
        // child
        if( pid == 0 )
        {
            execlp( buf, buf, (char *)0 );

            impossible( "could not execute %s", buf );    
        }
        
        // parent
        int ret = waitpid( pid, &status, 0 ); // 阻塞 等待子进程结束
        check( ret >= 0, "waitpid error");
        printf("%% ");
    }
    return 0;
    
error:
    return 1;
}