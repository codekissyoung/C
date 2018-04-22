#include "common.h"

int total_words;

void *count_words( void *f );

pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;

int main( int argc, char *argv[] )
{
	pthread_t t1;
	pthread_t t2;
	
	if( argc != 3 )
	{
		printf("输入两个文件");
		exit( 1 );
	}

	total_words = 0;

	pthread_create( &t1, NULL, count_words, (void*)argv[1]);
	pthread_create( &t2, NULL, count_words, (void*)argv[2]);

	pthread_join( t1, NULL );
	pthread_join( t2, NULL );

	printf("%5d : total_words\n", total_words );
	return 0;
}

void *count_words( void *f )
{
	char *filename = (char*)f;

	FILE *fp;

	int c;
	int prevc = '\0';

	if( ( fp = fopen( filename, "r" ) ) != NULL )
	{
		while( ( c = getc(fp) ) != EOF )
		{
			if( !isalnum( c ) && isalnum( prevc ) )
			{
				pthread_mutex_lock( &counter_lock );
				total_words ++;
				pthread_mutex_unlock( &counter_lock );
			}
			prevc = c;
		}
		fclose( fp );
	}
	else
	{
		perror( filename );
	}
	return NULL;
}
