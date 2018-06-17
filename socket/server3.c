#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct sockaddr_in SOCK_ADDRESS;
#define true 1
#define false 0
int main()
{
    int server_sockfd;
    int client_sockfd;
    int client_len;
    SOCK_ADDRESS server_address;
    SOCK_ADDRESS client_address;

    server_sockfd = socket( PF_INET, SOCK_STREAM, 0 );

    server_address.sin_family      = PF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port        = htons(10002);
    bind( server_sockfd, (struct sockaddr *)&server_address, sizeof(server_address) );

    listen( server_sockfd, 5 );
    signal( SIGCHLD, SIG_IGN );

    while(true)
    {
        char ch;
        printf("server waiting\n");
        client_len = sizeof(client_address);
        client_sockfd = accept( server_sockfd, (struct sockaddr *)&client_address, &client_len );
        if( fork() == 0 )
        {
            read( client_sockfd, &ch, 1 );
            sleep( 5 );
            ch++;
            write( client_sockfd, &ch, 1 );
            close( client_sockfd );
            exit(0);
        }
        else
        {
            close( client_sockfd );
        }
    }
}
