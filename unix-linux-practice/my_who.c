#include <stdio.h>
#include <utmp.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define SHOWHOST

void show_info( struct utmp * );

int main( int argc, char *argv[] )
{
    struct utmp current_record;
    int utmpfd;
    int reclen = sizeof(current_record);

    if( (utmpfd = open( UTMP_FILE, O_RDONLY )) == -1 )
    {
        perror( UTMP_FILE );
        return 1;
    }

    while( read( utmpfd, &current_record, reclen ) == reclen )
        show_info( &current_record );

    close( utmpfd );
    return 0;
}

void show_info( struct utmp *data )
{

    if( data -> ut_type == USER_PROCESS )
    {
        time_t t = data -> ut_time;
        printf("%s\t %s\t %12.12s\t %s\n",data->ut_name,data->ut_line,ctime( &t ) + 4,data->ut_host);
    }
}

