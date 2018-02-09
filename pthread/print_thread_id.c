#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void* thfn( void* arg )
{
    pid_t     pid;
    pthread_t tid;

    pid = getpid();
    tid = pthread_self();

    printf( "pid %u , tid : %u \n", (unsigned int)pid, (unsigned int)tid );
    return NULL;
}

int main( int argc, char* argv[] )
{

    pid_t pid;
    int err;
    pthread_t tid, mtid, ntid;

    pid  = getpid();
    mtid = pthread_self(); // 主线程ID

    err = pthread_create( &ntid, NULL, thfn, NULL );

    if( err != 0 )
    {
        printf("can not create thread %d \n", err );
        exit(1);
    }

    sleep( 1 );

    printf( "pid : %u , tid : %u \n", (unsigned int)pid, (unsigned int)mtid );
    return 0;
}
