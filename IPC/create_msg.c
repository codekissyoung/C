#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>

int main( int argc, char* argv[] )
{
    int qid;
    key_t key;

    key = 113;

    qid = msgget( key, IPC_CREAT | 0666 ); // 创建一个消息队列

    if( qid < 0 )
    {
        perror( "msgget" );
        exit(1);
    }

    printf( "created queue id : %d \n", qid );

    system("ipcs -q");
    exit( 0 );
}

