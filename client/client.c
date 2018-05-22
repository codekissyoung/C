#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>

int main ( int argc, char *argv[] )
{
    const char* ptr = "www.codekissyoung.com";
    char **pptr;
    struct hostent *hptr;
    char str[32];

    if( ( hptr = gethostbyname( (const char *)ptr ) ) == NULL )
    {
        printf(" gethostbyname error for host : %s \n ", ptr);
        exit( errno );
    }

    // 获取服务器域名对应的 第一个ip
    const char *server_host =  inet_ntop( hptr -> h_addrtype, hptr -> h_addr, str, sizeof( str ) );
    const int   server_port = 2046;

    // 客户端进程
    int sock = socket( AF_INET, SOCK_STREAM, 0 );

    // 初始化要连接的服务端地址
    struct sockaddr_in addr;
    memset( &addr, 0, sizeof( addr ) );
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr( server_host );
    addr.sin_port = htons( server_port );

    // 向服务端发起请求
    connect( sock, ( struct sockaddr* )&addr, sizeof( addr ) );

    char buffer[40];
    char read_str[100];
    while( 1 )
    {
        printf("(client)>");
        scanf("%s",read_str);
        printf( "you send : %s", read_str );
        write( sock, read_str, sizeof( read_str ) );

        read( sock, buffer, sizeof(buffer) - 1 );
        printf( "\n Message form server : %s \n", buffer );
    }
    close( sock );

    return 0;
}

