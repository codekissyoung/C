#include "common.h"

int total_words;
struct arg_set{
	char *fname;
	int count;
};

void *count_words( void *f );

pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;

int main( int argc, char *argv[] )
{
	if( argc != 3 )
	{
		printf("输入两个文件");
		exit( 1 );
	}
	
	total_words = 0;
	
	// 线程 1
	pthread_t t1;
	struct arg_set arg1;
	arg1.fname = argv[1];
	arg1.count = 0;
	pthread_create( &t1, NULL, count_words, (void*)&arg1 );
	
	// 线程 2	
	pthread_t t2;
	struct arg_set arg2;	
	arg2.fname = argv[2];
	arg2.count = 0;
	pthread_create( &t2, NULL, count_words, (void*)&arg2 );
	
	
	// 分别阻塞等待两线程返回
	pthread_join( t1, NULL );
	pthread_join( t2, NULL );

	printf("%d , %d , total words : %5d \n", arg1.count , arg2.count, arg1.count + arg2.count );
	return 0;
}

void *count_words( void *f )
{
	struct arg_set *arg = f;
	FILE   		   *fp;
	int    		   c;
	int            prevc = '\0';

	if( ( fp = fopen( arg -> fname, "r" ) ) != NULL )
	{
		while( ( c = getc(fp) ) != EOF )
		{
			if( !isalnum( c ) && isalnum( prevc ) )
			{
				arg -> count ++;
			}
			prevc = c;
		}
		fclose( fp );
	}
	else
	{
		perror( arg -> fname );
	}
	return NULL;
}
