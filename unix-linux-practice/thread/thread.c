#include "common.h"

#define NUM 5

void *print_msg( void *m );

int main( int argc, char *argv[] )
{
	pthread_t t1;
	pthread_t t2;

	pthread_create( &t1, NULL, print_msg, (void*)"hello");
	pthread_create( &t1, NULL, print_msg, (void*)"world\n");
	
	pthread_join( t1, NULL );
	pthread_join( t2, NULL );
}

void *print_msg( void *m )
{
	char *cp = (char*)m;
	int i;

	for( i = 0; i< NUM; i++ )
	{
		printf( "%s", cp );
		fflush( stdout );
		sleep( 1 );
	}

	return NULL;
}
