#include "common.h"

#define DELAY 10

void child_code( int delay );
void parent_code( int childpid );

int main( int argc, char *argv[] )
{
    int newpid;
    printf("before: mypid is %d \n", getpid() );
    if( ( newpid = fork() )  == - 1 ) 
        perror("fork error");
    else if( newpid == 0 )
        child_code( DELAY );
    else
        parent_code( newpid );

    return 0;
}

void child_code( int delay )
{
    printf("child %d here,sleep for %d seconds\n", getpid(), delay );
    sleep( delay );
    exit( 11 );
}

void parent_code( int childpid )
{
    int wait_rv;
    int status;
    int high_8;
    int low_7;
    int bit_7;
    wait_rv = wait( &status );
    
    high_8 = status >> 8;
    low_7  = status & 0x7F;
    bit_7  = status & 0x80;

    printf("done waiting for %d ,Wait returned : %d, status : %d, exit : %d, sig : %d, core : %d\n", 
            childpid, wait_rv, status, high_8, low_7, bit_7 );
}
