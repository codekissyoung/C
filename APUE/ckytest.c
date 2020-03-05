#include "common.h"
#include "dbg.h"
#include <pthread.h>

void *threadfunc( void *arg )
{
    sleep(5);
    char *s = (char *)arg;
    printf("%s", s);
    pthread_exit( (void*)strlen(s) );
}

int main( int argc, char *argv[])
{
    pthread_t t1;
    int ret = pthread_create( &t1, NULL, threadfunc, "Hello thread\n" );
    check( ret == 0, "thread create error");

    {
        void *res;
        int ret = pthread_join(t1, &res);
    }

    return 0;

error:
    return 1;
}