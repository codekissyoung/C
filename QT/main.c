#include <stdio.h>
#include <utmp.h>
#include <fcntl.h>
#include <unistd.h>
#include "func.h"

#define SHOWHOST
char (*(*X())[])();
void (*b[10])(void(*)());

void show_info(struct utmp *a)
{
    printf( "a->ut_user : %s\n", a->ut_user );
}

int main( int argc, char *argv[] )
{

    printf("argc: %d \n",argc);
    for(int i = 0; i < argc; i++ )
    {
        printf("argv[%d]:%s\n", i, argv[i]);
    }
    print_hello();

    struct utmp current_record;
    int utmpfd;
    int reclen = sizeof(current_record);

    if( (utmpfd = open(UTMP_FILE,O_RDONLY)) == -1 )
        perror(UTMP_FILE);

    while (read(utmpfd,&current_record,reclen) == reclen)
        show_info(&current_record);

    close(utmpfd);
    return 0;
}
