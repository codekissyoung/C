#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct sockaddr_un SOCK_ADDRESS;
#define true 1
#define false 0
int main()
{
    int server_sockfd;
    int client_sockfd;
    int client_len;
    SOCK_ADDRESS server_address;
    SOCK_ADDRESS client_address;

    unlink("server_socket");
    server_sockfd = socket(AF_UNIX,SOCK_STREAM,0);
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path,"server_socket");
    bind( server_sockfd, (struct sockaddr *)&server_address, sizeof(server_address) );

    listen(server_sockfd,5);

    while(true)
    {
        char ch;
        printf("server waiting\n");
        client_len = sizeof(client_address);
        client_sockfd = accept( server_sockfd, (struct sockaddr *)&client_address, &client_len );
        read( client_sockfd, &ch, 1 );
        ch++;
        write( client_sockfd, &ch, 1 );
        close(client_sockfd);
    }
    exit(0);
}
