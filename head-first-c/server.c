#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "server.h"

int main( int argc, char *argv[] )
{
    char *advice[] = {
        "less is more\r\n",
        "hard work,you will success\r\n",
        "Just for today, be honest\r\n",
        "You might want to rethink that haircut\r\n"
    };

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

    int reuse = 1;
    if( setsockopt( listener_d, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(int) ) == -1 )
    {
        printf("无法设置套接字的 重新使用端口 选项");
        exit(2);
    }


    if( listen(listener_d, 10) == -1 )
    {
        printf("无法监听");
        exit(1);
    }

    while( 1 )
    {
        struct sockaddr_storage client_addr;

        unsigned int address_size = sizeof(client_addr);

        int connect_d = accept( listener_d, (struct sockaddr *) &client_addr, &address_size );

        char *msg = advice[rand() % 5];

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

        close( connect_d );
    }

    return 0;
}

int read_in( int socket, char *buf, int len )
{
    char *s = buf;
    int slen = len;
    int c = recv(socket, s, slen, 0);

    while( ( c > 0) && ( s[c-1] != '\n' ) )
    {
        s += c;
        slen -= c;
        c = recv( socket, s, slen, 0 );
    }

    if( c < 0 )
        return c;
    else if( c == 0 )
        buf[0] = '\0';
    else
        s[c-1] = '\0';

    return len - slen;
}








