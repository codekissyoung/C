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
    int     backlog = atoi(argv[3]);

    int     sock    = socket(PF_INET, SOCK_STREAM, 0);
    check(sock > 0, "socket init error");

    struct sockaddr_in address;
    memset( &address, 0, sizeof(address) );
    address.sin_family = AF_INET;

    inet_pton( AF_INET, ip, &address.sin_addr );
    address.sin_port = htons(port);

    int ret = bind( sock,  (struct sockaddr* )&address, sizeof(address) );
    check(ret != -1, "bind error");

    ret = listen( sock, backlog );

    while( !stop ){
        sleep(1);
    }

    close(sock);

    return 0;

error:
    return 1;
}