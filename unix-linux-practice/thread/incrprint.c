#include "common.h"
#define NUM 5

int counter = 0;
void *print_count( void *m );

int main( int argc, char *argv[] )
{
	pthread_t t1;
	int i;

	pthread_create( &t1, NULL, print_count, NULL );	
	for( i = 0; i < NUM; i++ )
	{
		counter ++;
		sleep( 1 );
	}
	pthread_join( t1, NULL );
}

void *print_count( void *m )
{
	for( int i = 0; i < NUM; i++ )
	{
		printf( "count = %d \n", counter );
		sleep(1);
	}
	return NULL;
}
