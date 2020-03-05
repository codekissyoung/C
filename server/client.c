#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include "dbg.h"

#define MAX_LINE    (1024)
#define SERVER_PORT (12345)

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef struct epoll_event epoll_event;

int main(int argc, char *argv[])
{
    int  sockfd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_addr;
    memset( &server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    connect(sockfd, (sockaddr*)&server_addr, sizeof(server_addr));

    char input[100] = {"hello server"};
    
    strcat(input, argv[1]);

    // char recvline[MAX_LINE + 1] = {0};
    while( 1 )
    {
        int send_cnt = send( sockfd, input, strlen(input), 0 );
        log_info("send %d ", send_cnt);
        sleep(5);
    }

    close( sockfd );

    return 0;
}
