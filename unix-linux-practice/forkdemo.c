#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main( int argc, char *argv[] )
{
    printf("Before PID : %d\n",getpid());
    fork();
    fork();
    fork();
    printf("After PID : %d\n",getpid());
    return 0;
}
