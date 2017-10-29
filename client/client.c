#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main ( int argc, char *argv[] )
{
    int sock = socket( AF_INET, SOCK_STREAM, 0 );

    // 向服务端发起请求
    struct sockaddr_in addr;
    memset( &addr, 0, sizeof( addr ) );
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr( "101.200.144.41" );
    addr.sin_port = htons( 2046 );
    connect( sock, ( struct sockaddr* )&addr, sizeof( addr ) );

    char buffer[40];
    read( sock, buffer, sizeof(buffer) - 1 );

    printf( "Message form server : %s \n", buffer );

    close( sock );

    return 0;
}
