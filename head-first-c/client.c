#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

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

    char buf[255];
    sprintf( buf, "GET / http/1.1\r\n" );
    say( d_sock, buf );
    say( d_sock, "Host: en.wikipedia.org\r\n\r\n" );

    char rec[256];
    int bytes_rcvd = recv( d_sock, rec, 255, 0 );
    while( bytes_rcvd )
    {
        if( bytes_rcvd == -1 )
            error("Can not read from server");

        rec[bytes_rcvd] = '\0';
        printf("%s",rec);

        bytes_rcvd = recv( d_sock, rec, 255, 0 );
    }

    close(d_sock);
    return 0;
}
