#include <stdio.h>
#include <utmp.h>
#include <fcntl.h>
#include <unistd.h>
#define SHOWHOST

void show_info(struct utmp *a)
{
    printf("%s\t%s\t%s\t%s\n",a->ut_name,a->ut_line,a->ut_time,a->ut_host);
}

int main(int argc, char *argv[])
{
    struct utmp current_record;
    int utmpfd;
    int reclen = sizeof(current_record);

    if( utmpfd = open(UTMP_FILE,O_RDONLY) == -1 ){
        perror(UTMP_FILE);
    }

    while (read(utmpfd,&current_record,reclen) == reclen)
        show_info(&current_record);

    close(utmpfd);
}
