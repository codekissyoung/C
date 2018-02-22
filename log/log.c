#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
int main()
{
    FILE *file;
    file = fopen( "not_a_file", "r" );
    if( !file )
    {
        syslog(LOG_ERR|LOG_USER,"oops - %m\n");
    }
    exit(0);
}
