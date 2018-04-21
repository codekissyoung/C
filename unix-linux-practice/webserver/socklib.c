#include "common.h"
#define HOSTLEN 256
#define BACKLOG 1

int make_server_socket_q( int , int );
int make_server_socket( int portnum )
{
	return make_server_socket_q( portnum, BACKLOG );
}

int make_server_socket_q( int portnum, int backlog )
{
	struct sockaddr_in saddr;
	struct hostent *hp;
	char hostname[HOSTLEN];
	int sock_id;
	
	sock_id = socket( PF_INET, SOCK_STREAM, 0 );


}
