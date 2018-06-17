#include "common.h"

#define oops( m, x ) { perror(m); exit(x); }

int main( int argc, char *argv[] )
{
	int 			fd1;
	int				fd2;
	int 			maxfd;
	int 			retval;
	fd_set 			readfds;
	struct timeval	timeout;

	if( argc != 4 )
	{
		fprintf(stderr,"usage : %s 要4个参数 \n", argv[0] );
		exit(12);
	}

	fd1 = open( argv[1], O_RDONLY );
	fd2 = open( argv[2], O_RDONLY );

	maxfd = 1 + ( fd1 > fd2 ? fd1 : fd2 );

	while( 1 )
	{
		FD_ZERO( &readfds );
		FD_SET( fd1, &readfds );
		FD_SET( fd2, &readfds );

		timeout.tv_sec  = atoi( argv[3] );
		timeout.tv_usec = 0;

		retval = select( maxfd, &readfds, NULL, NULL, &timeout );
		if( retval == -1 )
		{
			oops( "select", 4 );
		}
		if( reval > 0 )
		{
			if( FD_ISSET( fd1, &readfds ) )
				showdata( argv[1], &readfds );
			if( FD_ISSET( fd2, &readfds ) )
				showdata( argv[2], &readfds );
		}
		else
		{
			printf("no input after %d seconds\n",atoi(argv[3]) );
		}
	}
}

showdata( char *fname, int fd )
{
	char buf[BUFSIZE]
	int n;
}
