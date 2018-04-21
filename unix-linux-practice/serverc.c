#include "common.h"
#define PORTNUM 13000
#define HOSTLEN 256
#define oops(msg) {perror(msg);exit(1);}

int main( int argc, char *argv[] )
{
	struct sockaddr_in saddr;
	struct hostent *hp;

	char hostname[HOSTLEN];
	int sock_id,sock_fd;

	FILE *sock_fp;
	char *ctime();
	time_t thetime;

	sock_id = socket( PF_INET, SOCK_STREAM, 0 );

	bzero( (void*)&saddr, sizeof(saddr) );


}
