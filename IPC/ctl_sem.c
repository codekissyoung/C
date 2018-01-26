#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <unistd.h>

int main( int argc, char* argv[] )
{
    int semid;
    int nsems = 1;
    int flags = 0666;

    semid = semget( IPC_PRIVATE, nsems, flags );

    printf( "successfully created a semaphore : %d\n",semid );

    system("ipcs -s ");

    if( semctl( semid, 0, IPC_RMID) < 0 )
    {
        perror("semctl");
        exit(1);
    }
    else
    {
        printf("semaphore removed \n");
        system("ipcs -s");
    }
    exit(0);
}




