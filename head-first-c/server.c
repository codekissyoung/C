#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "server.h"

int main( int argc, char *argv[] )
{
    int listener_d = socket( PF_INET, SOCK_STREAM, 0 );
    if( listener_d == -1 )
    {
        printf("socket error");
        exit(1);
    }

    struct sockaddr_in name;

    name.sin_family = PF_INET;
    name.sin_port = (in_port_t)htons(30000);
    name.sin_addr.s_addr = htonl(INADDR_ANY);

    int c = bind(listener_d, (struct sockaddr *) &name, sizeof(name) );
    if( c == -1 )
    {
        printf("无法绑定端口");
        exit(1);
    }

    if( listen(listener_d, 10) == -1 )
    {
        printf("无法监听");
        exit(1);
    }

    struct sockaddr_storage client_addr;

    unsigned int address_size = sizeof(client_addr);

    int connect_d = accept( listener_d, (struct sockaddr *) &client_addr, &address_size );

    char *msg = "Internet Knock-Knock Protocol Server\r\nVersion 1.0\r\nKnock! Knock!\r\n";

    if( send(connect_d, msg, strlen(msg), 0) == -1 )
    {
        printf("send");
        exit(1);
    }

    if( connect_d == -1 )
    {
        printf("无法打开副套接字");
        exit(1);
    }

    printf("server off\n");
    return 0;
}
