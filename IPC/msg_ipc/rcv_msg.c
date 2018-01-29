#include "common.h"
int main( int argc, char* argv[] )
{
    int qid;
    int len;
    struct msg pmsg;

    if( argc != 2 )
    {
        perror("USAGE : read_msg <queue ID>");
        exit(1);
    }

    qid = atoi( argv[1] );

    len = msgrcv( qid, &pmsg, BUFSZ, 0, 0 );

    if( len > 0 )
    {
        pmsg.msg_buf[len] = '\0';
        printf("reading queue id : %d\n", qid );
        printf("message type : %ld\n", pmsg.msg_types );
        printf("message length : %d types\n", len);
        printf("message text : %s\n", pmsg.msg_buf );
    }
    else if( len == 0 )
    {
        printf("have no message from queue %d\n", qid );
        exit(1);
    }
    else
    {
        perror("msgrcv error");
        exit(1);
    }

    system("ipcs -q");
    exit(0);
}
