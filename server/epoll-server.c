#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <sys/epoll.h>
#include "dbg.h"

#define SERVER_PORT     (12345)
#define EPOLL_MAX_NUM   (2048)
#define BUFFER_MAX_LEN  (1024)
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef struct epoll_event epoll_event;

void str_toupper(char *str);
int  init_listen_socket(short port);

void epoll_add_read_event_fd_lt( int epoll_fd, int fd );
void epoll_add_read_event_fd_et( int epoll_fd, int fd );
void epoll_remove_read_event_fd_lt( int epoll_fd, int fd );
void epoll_remove_read_event_fd_et( int epoll_fd, int fd );
int set_nonblocking( int fd );

int main(int argc, char *argv[])
{
    int epoll_fd  = epoll_create(EPOLL_MAX_NUM);
    int listen_fd = init_listen_socket(SERVER_PORT);

    epoll_add_read_event_fd_lt( epoll_fd, listen_fd );
    epoll_event *my_events = malloc(sizeof(epoll_event) * EPOLL_MAX_NUM);
    
    while( 1 )
    {
        log_info("server [%d] wait for client : ...", listen_fd);
        int active_fds_cnt = epoll_wait(epoll_fd, my_events, EPOLL_MAX_NUM, -1);
        for( int i = 0; i < active_fds_cnt; i++ )
        {
            if( listen_fd == my_events[i].data.fd )
            {
                sockaddr_in client_addr;
                socklen_t   client_len = sizeof(client_addr);
                int client_fd = accept(listen_fd, (sockaddr*)&client_addr, &client_len);
                #ifdef DEBUG
                char ip[20];
                log_info("new connection %d [%s:%d]",
                         client_fd,
                         inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip)), 
                         ntohs(client_addr.sin_port));
                #endif
                epoll_add_read_event_fd_et( epoll_fd, client_fd );
            }
            else if( my_events[i].events & EPOLLIN )
            {
                int client_fd = my_events[i].data.fd;
                #ifdef DEBUG
                log_info("client [%d] can read", client_fd);
                #endif

                char buffer[BUFFER_MAX_LEN];
                memset(buffer, '\0', sizeof(buffer));
                int data_cnt = 0;
                while( 1 )
                {
                    int recv_cnt = recv( client_fd, buffer + data_cnt, 10, 0 );
                    if( recv_cnt < 0 )
                    {
                        if( errno == EAGAIN || errno == EWOULDBLOCK )
                        {
                            log_info("%d read later", client_fd);
                            break;
                        }
                    }
                    else if( recv_cnt == 0 )
                    {
                        epoll_remove_read_event_fd_et(epoll_fd, client_fd);
                        shutdown(client_fd, SHUT_RD);
                        log_info("client %d closed", client_fd);
                        break;
                    }
                    else
                    {
                        data_cnt += recv_cnt;
                        log_info("client [%d] read %d : %s", client_fd, recv_cnt, buffer);
                    }
                }
            }
            else if( my_events[i].events & EPOLLOUT )
            {
                int client_fd = my_events[i].data.fd;
                #ifdef DEBUG
                log_info("client [%d] can write", client_fd);
                #endif
            }
        }
    }
    close(listen_fd);
    close(epoll_fd);
    return 0;
}

void epoll_add_read_event_fd_lt( int epoll_fd, int fd )
{
    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    epoll_ctl( epoll_fd, EPOLL_CTL_ADD, fd, &event );
}

void epoll_remove_read_event_fd_lt( int epoll_fd, int fd )
{
    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    epoll_ctl( epoll_fd, EPOLL_CTL_DEL, fd, &event );
}

void epoll_add_read_event_fd_et( int epoll_fd, int fd )
{
    set_nonblocking( fd );
    epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = fd;
    epoll_ctl( epoll_fd, EPOLL_CTL_ADD, fd, &event );
}

void epoll_remove_read_event_fd_et( int epoll_fd, int fd )
{
    epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = fd;
    epoll_ctl( epoll_fd, EPOLL_CTL_DEL, fd, &event );
}

int init_listen_socket( short port )
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    #ifdef DEBUG
    int reuse = 1;
    setsockopt( listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof( reuse ) );
    #endif
    sockaddr_in server_addr;
    memset( &server_addr, 0, sizeof(server_addr) );
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons( port );

    bind(listen_fd, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(listen_fd, 10);
    return listen_fd;
}

void str_toupper(char *str)
{
    for(int i; i < strlen(str); i++)
        str[i] = toupper(str[i]);
}

int set_nonblocking( int fd )
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}