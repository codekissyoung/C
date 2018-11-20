#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include "common.h"
#include "client.h"

void error( char *s )
{
    fprintf( stderr, "%s : %s", s, strerror( errno ) );
    exit(1);
}

int open_socket( char *host, char *port )
{
    struct addrinfo *res;
    struct addrinfo hints;

    memset( &hints, 0, sizeof(hints) );

    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if( getaddrinfo(host,port,&hints,&res) == -1 )
        error("Can not resolve the address");
    
    int d_sock = socket( res->ai_family, res->ai_socktype, res->ai_protocol );

    if( d_sock == -1 )
        error("Cant not open socket");

    int c = connect( d_sock, res->ai_addr, res->ai_addrlen );

    freeaddrinfo(res);

    if( c == -1 )
        error("Cant not connect to socket");

    printf("Client is connecting\n");

    return d_sock;
}

int say( int socket, char *s )
{
    int result = send( socket, s, strlen(s), 0 );
    if( result == -1 )
        fprintf(stderr, "%s : %s\n","Error talking to server", strerror(errno) );
    return result;
}

int main( int argc, char const *argv[] )
{
    int d_sock;
    d_sock = open_socket("codekissyoung.com","30000");

    if( argc < 2 )
    {
        printf("client arg1");
    }
    int tmpLen = strlen(argv[1]);

    Node    *dataBuf    = (Node*)malloc( sizeof(Node) + tmpLen + 1 );
    dataBuf -> bufSize  = tmpLen + 1;
    dataBuf -> nodeSize = sizeof(Node) + dataBuf -> bufSize;

    memset( dataBuf->buf, 0, dataBuf -> bufSize );
    memmove( dataBuf->buf, argv[1], dataBuf -> bufSize );

    printf("nodeSize = %d\n bufSize = %d\n buf = %s\n",dataBuf->nodeSize,dataBuf->bufSize,dataBuf->buf);

    if( send( d_sock, (char*)dataBuf, dataBuf->nodeSize, 0 ) == -1 )
        error("send error!\n");

    close(d_sock);
    return 0;
}
