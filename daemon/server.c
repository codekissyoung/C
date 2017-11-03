#ifndef TLPI_HDR_H
#define TLPI_HDR_H
    #include <sys/types.h>
    #include <sys/param.h>
    #include <sys/stat.h>
    #include <sys/socket.h>

    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <signal.h>
    #include <time.h>
    #include <string.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <errno.h>
    #include <sys/wait.h>
    #include <gnu/libc-version.h>

    typedef enum { FALSE, TRUE } Boolean;
    #define min(m,n) ((m) < (n) ? (m) : (n))
    #define max(m,n) ((m) > (n) ? (m) : (n))

    #include "get_num.h"
    #include "error_functions.h"
#endif

void init_daemon();
void daemon_log( char* str );

int main( int argc, char *argv[] )
{
    printf( "libc version : %s \n", gnu_get_libc_version());
    // 将进程变为守护进程
    init_daemon();

    // 创建套接字
    int sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    // 绑定 ip 以及 port
    struct sockaddr_in addr;
    memset( &addr, 0, sizeof(addr) ); // 地址全部填充 0

    addr.sin_family         = AF_INET;
    addr.sin_addr.s_addr    = inet_addr( "0.0.0.0" );
    addr.sin_port           = htons( 2046 );

    bind( sock, (struct sockaddr*) &addr, sizeof( addr ) );

    // 开始监听
    listen( sock, SOMAXCONN );

    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof( client_addr );
    char str[40];
    while( 1 )
    {
        int client_sock = accept( sock, ( struct sockaddr* )&client_addr, &client_addr_size );
        daemon_log("新建立socket");

        // 创建一个子进程处理该次请求
        pid_t handler_pid = fork();
        if( handler_pid < 0 )
        {
            exit( errno );
        }
        // 父进程( 即守护进程 )记录日志
        else if( handler_pid > 0 )
        {
            // 避免子进程变成僵尸进程
            waitpid( handler_pid, NULL, 0 );
        }
        else
        {
            // handler 进程处理请求 , 向客户端发送数据
            while( 1 )
            {
                if ( read( client_sock, str, sizeof( str ) - 1 ) != -1 )
                {
                    daemon_log( str );
                    if( sizeof(str) > 10 )
                    {
                        if ( write( client_sock, str, sizeof( str ) ) == -1 )
                        {
                            daemon_log( "写入 client_sock 失败" );
                        }
                        daemon_log( strcat( str, "xxxx" ) );
                        if( strcmp( "stop", str) == 0 )
                        {
                            // 直接调用close()对本端进行关闭仅仅使socket的引用计数减1，socket并没关闭。
                            // 从而导致系统中又多了一个CLOSE_WAIT的socket, 正确关闭应该先 shutdown
                            shutdown( client_sock, SHUT_RDWR );
                            close( client_sock );
                            exit( 0 );
                        }
                    }
                }
            }
        }
    }

    // 服务端关闭套接字
    close( sock );
    return 0;
}

// 打印服务器系统日志
void daemon_log( char* str )
{
    FILE *fp;
    time_t t;
    fp = fopen( "sys-time.log", "a" );
    if( fp )
    {
        time( &t );
        fprintf( fp, "%s  %s \n", asctime( localtime( &t ) ), str );
        fclose( fp );
    }
}

/*
 * 守护进程
 * 特性:
 * 1. 后台运行
 * 2. 与运行前环境( 运行守护进程的终端环境 | /etc/rc.d 脚本 | crond 脚本 ) 完全脱离 , 不使用继承而来的 文件描述符 , 控制终端
 *    会话Session , 进程组 , 工作目录 , 文件创建掩模.
 * 编写守护进程 就是将一个普通进程 按照上述特性 改造成 守护进程.
 * */
void init_daemon()
{
    int pid;

    // 创建子进程
    pid = fork();

    // 子进程创建错误 退出
    if( pid < 0 )
        exit( 1 );

    // 进程创建成功 父进程正常退出
    else if( pid > 0 )
    {
        printf("Parent process exit\n");
        exit( 0 );
    }

    printf("child process run\n");

    // 子进程成为 新进程组长 以及 新会话组长 ，并且与 原会话 和 进程组 脱离
    setsid();

    // 子进程再创建子进程 , 下称为 子子进程
    pid = fork();

    // 子子进程创建错误 退出
    if( pid < 0 )
        exit( 1 );

    // 子进程正常退出
    else if( pid > 0 )
        exit( 0 );

    // 以下都是 子子进程 的执行代码了
    // 子子进程 不是 进程组长 所以不会再打开控制终端

    // 关闭 子子进程 继承而来的所有文件句柄
    for( int i = 0; i < NOFILE; i++ )
        close( i );

    // 修改 子子进程 的工作目录
    if( chdir( "./" ) == -1 )
    {
        daemon_log( "修改工作目录失败" );
        exit( errno );
    }

    // 重新设置 子子进程 的 文件掩码 , 不使用从父进程继承来的
    umask( 0 );
    daemon_log("daemon process run !!!\n");
}


