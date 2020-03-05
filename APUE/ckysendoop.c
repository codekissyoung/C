#include "common.h"
#include "dbg.h"

static bool stop = FALSE;

static void handle_term(int sig){
    stop = TRUE;
}

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

    int ret = connect( sockfd, (struct sockaddr*)&server_address, sizeof(server_address) );
    check( ret >= 0, "connect error");

    char *oob_data = "abc";
    char *normal_data = "123";
    send( sockfd, normal_data, strlen(normal_data), 0 );
    send( sockfd, oob_data, strlen(oob_data), MSG_OOB );
    send( sockfd, normal_data, strlen(normal_data), 0 );

    close( sockfd );

    return 0;

error:
    return 1;
}