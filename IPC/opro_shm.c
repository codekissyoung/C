#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main( int argc, char* argv[] )
{
    int shm_id;
    char* shm_buf;

    shm_id = atoi( argv[1] );

    if( (shm_buf = shmat( shm_id, 0, 0 ) ) < (char *)0 )
    {
        perror( "shmat");
        exit(1);
    }

    printf( "segment attachted at %p \n", shm_buf );
    system( "ipcs -m" );

    sleep( 3 );

    if( shmdt(shm_buf) < 0 )
    {
        perror("shmdt error\n");
        exit(1);
    }

    printf( "Segment detached \n" );
    system( "ipcs -m" );

    exit( 0 );
}

