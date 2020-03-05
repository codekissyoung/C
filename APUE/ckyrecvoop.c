#include "common.h"
#include "dbg.h"

static bool stop = FALSE;

static void handle_term(int sig){
    stop = TRUE;
}
#define BUF_SIZE 1024
int main( int argc, char *argv[]){
    signal( SIGTERM, handle_term );

    char    *ip     = argv[1];
    int     port    = atoi(argv[2]);
    
    struct sockaddr_in server_address;
    memset( &server_address, 0, sizeof(server_address) );
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &server_address.sin_addr );
    server_address.sin_port = htons( port );
    
    int sockfd = socket( PF_INET, SOCK_STREAM, 0 );
    check( sockfd > 0, "socket init error");

    // 强制使用被 TIME_WAIT 状态占用的 socket 端口
    int reuse = 1;
    setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse) );

    int ret = bind( sockfd, (struct sockaddr*)&server_address, sizeof(server_address) );
    check(ret != -1, "bind port error");

    ret = listen( sockfd, 5 );
    check(ret != -1, "listen error");

    struct sockaddr_in client_addr;
    socklen_t client_addr_lenth = sizeof( client_addr );

    int connfd = accept( sockfd, (struct sockaddr*)&client_addr, &client_addr_lenth );
    if( connfd < 0 ){
        log_info("client accept failed!");
    }else{
        char buffer[BUF_SIZE];

        memset( buffer, '\0', BUF_SIZE );
        ret = recv( connfd, buffer, BUF_SIZE - 1, 0 );
        log_info("got %d bytes of normal data %s", ret, buffer ); 
        
        memset( buffer, '\0', BUF_SIZE );
        ret = recv( connfd, buffer, BUF_SIZE - 1, MSG_OOB );
        log_info("got %d bytes of oob data %s", ret, buffer ); 


        memset( buffer, '\0', BUF_SIZE );
        ret = recv( connfd, buffer, BUF_SIZE - 1, 0 );
        log_info("got %d bytes of normal data %s", ret, buffer ); 

        close(connfd);
    }
    close( sockfd );

    return 0;

error:
    return 1;
}