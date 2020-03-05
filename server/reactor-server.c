#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#include <sys/epoll.h>
#include "dbg.h"

#define SERVER_PORT     (12345)
#define EPOLL_MAX_NUM   (2048)
#define BUFFER_MAX_LEN  (1024)
#define MAX_EVENTS      (1024)
#define BUFLEN          (128)

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef struct epoll_event epoll_event;
typedef void(*event_call_back)(int fd, int events, void *arg);
typedef struct myevent_s{
    int fd;
    int events;
    void *arg;
    event_call_back call_back;
    int status;         // 1 在监听事件中；0 不在
    char buf[BUFLEN];
    int len;
    long last_active; // 最后一次响应的时间
} myevent_s;

int g_efd;

myevent_s g_events[MAX_EVENTS + 1]; // 队列，最后一个用于 list end fd

void str_toupper(char *str);
int  init_listen_socket(int epoll_id, short port);

void epoll_add_read_event_fd_lt( int epoll_fd, int fd );
void epoll_add_read_event_fd_et( int epoll_fd, int fd );
void epoll_remove_read_event_fd_lt( int epoll_fd, int fd );
void epoll_remove_read_event_fd_et( int epoll_fd, int fd );
int  set_nonblocking( int fd );
void eventset(myevent_s *ev, int fd, event_call_back, void *arg );
void eventadd(int efd, int events, myevent_s *ev);
void eventdel(int efd, myevent_s *ev);
void accept_handler(int lfd, int events, void *arg);
void recvdata(int fd, int events, void *arg);
void senddata(int fd, int events, void *arg);

int main(int argc, char *argv[])
{
    g_efd = epoll_create( MAX_EVENTS + 1 );
    init_listen_socket( g_efd, SERVER_PORT );
    return 0;
}

void eventset(myevent_s *ev, int fd, event_call_back func, void *arg )
{
    ev -> fd        = fd;
    ev -> call_back = func;
    ev -> events    = 0;
    ev -> arg       = arg;
    ev -> status    = 0;
    ev -> len       = 0;
    ev -> last_active = time(NULL);
    memset(ev->buf, 0, sizeof(ev -> buf));
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

int init_listen_socket( int epoll_fd, short port )
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    set_nonblocking( listen_fd );

    eventset( &g_events[MAX_EVENTS], listen_fd, accept_handler, &g_events[MAX_EVENTS] );
    eventadd( epoll_fd, EPOLLIN, &g_events[MAX_EVENTS] );

    sockaddr_in server_addr;
    memset( &server_addr, 0, sizeof(server_addr) );
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons( port );

    bind(listen_fd, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(listen_fd, 10);
    return listen_fd;
}

void eventadd(int epoll_fd, int events, myevent_s *ev)
{
    epoll_event event;
    event.events    = ev -> events = events;
    event.data.ptr  = ev;
    epoll_ctl( epoll_fd, EPOLL_CTL_ADD, ev->fd, &event );
}

void eventdel(int epoll_fd, myevent_s *ev)
{
    epoll_event event;
    event.data.ptr  = ev;
    ev->status      = 0;
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, ev->fd, &event);
}

void accept_handler(int listen_fd, int events, void *arg )
{
    sockaddr_in cin;
    socklen_t len = sizeof(cin);
    int conn_fd = accept(listen_fd, (sockaddr*)&cin, &len);
    int i = 0;
    for(i = 0; i < MAX_EVENTS; i++)
    {
        if( g_events[i].status == 0)
            goto handler_end;
    }
    if( i == MAX_EVENTS )
    {
        log_err("max connect limit : %d", i);
        goto handler_end;
    }

    eventset( &g_events[i], conn_fd, recvdata,  &g_events[i] );
    eventadd( g_efd, EPOLLIN, &g_events );
    
    handler_end:
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
