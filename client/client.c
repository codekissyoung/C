#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

int main ( int argc, char *argv[] )
{
    const char* ptr = "www.codekissyoung.com";
    char **pptr;
    struct hostent *hptr;
    char str[32];

    printf( "gethostbyname error for host : %s \n ", ptr );
    if( ( hptr = gethostbyname( (const char *)ptr ) ) == NULL )
    {
        printf(" gethostbyname error for host : %s \n ", ptr);
        return 0;
    }

    printf( "official hostname: %s \n", hptr->h_name );
    for( pptr = hptr->h_aliases; *pptr != NULL; pptr++ )
        printf( "alias:%s\n", *pptr );

    switch(hptr->h_addrtype)
    {
        case AF_INET:
        case AF_INET6:
            pptr = hptr -> h_addr_list;
            for( ; *pptr != NULL; pptr++ )
                printf( "address : %s \n ", inet_ntop( hptr -> h_addrtype, *pptr, str, sizeof(str) ) );
            printf( "first address: %s\n", inet_ntop( hptr->h_addrtype, hptr->h_addr, str, sizeof(str) ) );
        break;
        default:
            printf("unknown address type\n");
        break;
    }

    // 客户端进程
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
