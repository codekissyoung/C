#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <stdint.h>

#include "common.h"
#include "server.h"

int listener_d;
typedef struct sockaddr_storage sockaddr_storage;

void error( char *s );
void bind_to_port( int socket, int port );
void handler_shutdown( int sig );
void diediedie( int sig );

int say( int socket, char *s );
int read_in( int socket, char *buf, int len );
int catch_signal( int sig, void(*handler)(int) );
int open_listener_socket();

int main( int argc, char *argv[] )
{
    // 设置中断处理函数失败
    if( catch_signal( SIGINT, handler_shutdown ) == -1 )
        error("设置中断处理函数失败");

    // ------------------- 程序开始 ------------------- //
    listener_d = open_listener_socket();

    bind_to_port( listener_d, 30000 );

    if( listen( listener_d, 10 ) == -1 )
        error("Cant not listen");

    puts("Waiting for connection ...");

    sockaddr_storage client_addr;
    pid_t    pid;
    socklen_t address_size = sizeof(client_addr);

    while( 1 )
    {
        int connect_d = accept( listener_d, (struct sockaddr *)&client_addr, &address_size );
        if( connect_d == -1 )
            error("不能打开第二个socket");

        if( (pid = fork()) < 0 )
            error("fork error\n");
        
        // 父进程     
        if( pid > 0 )
        {
            close(connect_d);
        }

        // 子进程 
        else
        {
            close(listener_d);
            if( say( connect_d, "Server Connected!\n" ) != -1 )
            {
                int nodeSize = 0;
                int read_num = read_in( connect_d, (char*)&nodeSize, sizeof(nodeSize) );
                if( read_num < 0 )
                    error("read nodeSize error\n");

                Node *client_data = (Node*)malloc( nodeSize );
                client_data -> nodeSize = nodeSize;
                read_num = read_in( connect_d, (char*)&(client_data -> bufSize), nodeSize - sizeof(nodeSize) );
                if( read_num < 0 )
                    error("read Buffer error\n");
                printf("nodeSize = %d\n bufSize = %d\n buf = %s\n",client_data->nodeSize,client_data->bufSize,client_data->buf);
            }
            close(connect_d);
            exit(0);
        }
    }
    return 0;
}

// recv() 调用不一定一次调用就能接收到所有数据
// recv() 返回已接收的字符个数，发生错误则返回-1,客户端关闭了链接则返回0
int read_in( int socket, char *buf, int len )
{
    char *local_buff = buf;
    int local_len = len;
    int recv_len = 0;
    do{
        recv_len = recv( socket, local_buff, local_len, 0 );
        if( recv_len < 0 )
        {
            return recv_len;
        }
        if( recv_len == 0 )
        {
            return 0;
        }
        if( recv_len > 0 )
        {
            local_buff += recv_len;
            local_len -= recv_len;
        }
    }while(local_buff[-1] != '\n');
    return len - local_len;
}

void error( char *s )
{
    fprintf( stderr, "%s : %s", s, strerror( errno ) );
    exit(1);
}

int open_listener_socket()
{
    int s = socket( PF_INET, SOCK_STREAM, 0 );
    if( s == -1 )
        error("打开socket失败");
    return s;
}

void bind_to_port( int socket, int port )
{
    struct sockaddr_in name;
    name.sin_family      = PF_INET;
    name.sin_port        = (in_port_t)htons(30000);
    name.sin_addr.s_addr = htonl( INADDR_ANY );

    // 操作系统限制在某个端口绑定了套接字，在接下来30s内，不允许任何程序再绑定这个端口,通过下面这个设置，可以解除这个限制
    int reuse = 1;
    if( setsockopt( socket, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(int)) == -1 )
        error("Can not reuse option on socket");
    
    if( bind( socket, (struct sockaddr *)&name, sizeof(name) ) == -1 )
        error("Can not bind to socket");
}

int say( int socket, char *s )
{
    int result = send( socket, s, strlen(s), 0 );
    if( result == -1 )
        fprintf(stderr,"%s : %s \n","和客户端通信时发生了错误",strerror(errno));
    return result;
}

void diediedie( int sig )
{
    puts("\nGoodbye cruel world...\n");
    exit(1);
}

int catch_signal( int sig, void(*handler)(int) )
{
    struct sigaction action;
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    return sigaction(sig, &action, NULL);
}

void handler_shutdown( int sig )
{
    if(listener_d)
        close(listener_d);
    
    fprintf(stderr,"Bye!\n");
    exit(0);
}