#include "common.h"
#include "dbg.h"
#include <pthread.h>
#include <sys/epoll.h>

#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 10

int setnonblocking(int fd)
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}

void addfd( int epollfd, int fd, bool enable_et )
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if( enable_et ){
        event.events |= EPOLLET;
    }
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

void lt(struct epoll_event *events, int number, int epollfd, int listenfd)
{
    char buf[BUFFER_SIZE];
    for( int i = 0; i< number; i++ ){
        int sockfd = events[i].data.fd;
        if( sockfd == listenfd ){
            struct sockaddr_in client_address;
            socklen_t client_address_length = sizeof(client_address);
            int connfd = accept( listenfd, (struct sockaddr*)&client_address, &client_address_length);
            addfd( epollfd, connfd, FALSE);
        }
        else if( events[i].events & EPOLLIN ){
            log_info("event trigger once");
            memset(buf, '\0', BUFFER_SIZE);
            int ret = recv( sockfd, buf, BUFFER_SIZE - 1, 0);
            if( ret <= 0 ){
                close(sockfd);
                continue;
            }
            log_info("get %d bytes of contents: %s\n", ret, buf);
        }
        else{
            log_err("something else happened");
        }
    }
}

void et(struct epoll_event *events, int number, int epollfd, int listenfd)
{
    char buf[BUFFER_SIZE];
    for(int i = 0; i < number; i++ )
    {
        int sockfd = events[i].data.fd;
        if( sockfd == listenfd )
        {
            struct sockaddr_in client_address;
            socklen_t client_address_length = sizeof(client_address);
            int connfd = accept( listenfd, (struct sockaddr*)&client_address, &client_address_length);
            addfd( epollfd, connfd, TRUE);
        }
        else if( events[i].events & EPOLLIN )
        {
            log_info("event trigger once");
            while( TRUE )
            {
                memset( buf, '\0', BUFFER_SIZE);
                int ret = recv( sockfd, buf, BUFFER_SIZE - 1, 0 );
                if( ret < 0 )
                {
                    if( (errno == EAGAIN) || ( errno == EWOULDBLOCK))
                    {
                        log_info("read later");
                        break;
                    }
                    close(sockfd);
                    break;
                }              
                else if( ret == 0 )
                {
                    close( sockfd );
                }
                else
                {
                    log_info("get %d bytes of contents: %s", ret, buf);
                }
            }
        }
        else
        {
            printf("something else happened");
        }
    }
}

int main( int argc, char *argv[])
{
    char *ip = "127.0.0.1";
    int port = 12345;

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int listenfd = socket( PF_INET, SOCK_STREAM, 0 );
    check(listenfd >= 0, "init socket error");

    int ret = 0;
    ret = bind( listenfd, (struct sockaddr*)&address, sizeof(address) );
    check( ret != -1, "bind error");
    ret = listen(listenfd, 5);
    check( ret != -1, "listen error");

    struct epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    check( epollfd != -1, "epoll_create error");
    addfd( epollfd, listenfd, TRUE );

    while( TRUE )
    {
        int ret = epoll_wait( epollfd, events, MAX_EVENT_NUMBER, -1);
        check( ret >= 0, "epoll_wait error");

        // lt(events, ret, epollfd, listenfd);
        et(events, ret, epollfd, listenfd);
    }

    close(listenfd);
    return 0;

error:
    return 1;
}