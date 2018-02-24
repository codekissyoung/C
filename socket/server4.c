#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/ioctl.h>
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
    int server_len;
    int result;

    fd_set readfds,testfds;

    SOCK_ADDRESS server_address;
    SOCK_ADDRESS client_address;

    server_sockfd = socket( PF_INET, SOCK_STREAM, 0 );

    server_address.sin_family      = PF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port        = htons(10002);
    bind( server_sockfd, (struct sockaddr *)&server_address, sizeof(server_address) );

    listen( server_sockfd, 5 );

    FD_ZERO( &readfds );
    FD_SET( server_sockfd, &readfds );
    while(true)
    {
        char ch;
        int fd;
        int nread;
        testfds = readfds;
        printf("server waiting\n");
        result = select( FD_SETSIZE, &testfds, (fd_set*)0, (fd_set *)0, (struct timeval *)0 );
        if( result < 1 )
        {
            perror("server4");
            exit(1);
        }
        // 在一个进程中，使用 select系统调用,对多个客户依次处理
        for( fd = 0; fd < FD_SETSIZE; fd++ )
        {
            if( FD_ISSET( fd, &testfds ) )
            {
                if( fd == server_sockfd )
                {
                    client_len = sizeof( client_address );
                    client_sockfd = accept( server_sockfd, (struct sockaddr *)&client_address, &client_len );
                    FD_SET( client_sockfd, &readfds );
                    printf( "adding client on fd %d\n", client_sockfd );
                }
                else
                {
                    ioctl( fd, FIONREAD, &nread );
                    if( nread == 0 )
                    {
                        close(fd);
                        FD_CLR( fd, &readfds );
                        printf("removing client on fd %d\n", fd );
                    }
                    else
                    {
                        read( fd, &ch, 1 );
                        sleep( 5 );
                        printf( "serving client on fd %d\n", fd );
                        ch++;
                        write( fd, &ch, 1 );
                    }
                }
            }
        }
    }
    return 0;
}
