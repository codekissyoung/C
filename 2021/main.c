#include "func.h"
#include <stdio.h>
#include <time.h>

int main(void)
{
    char buffer[80];
    time_t rawTime;
    time( &rawTime );
    strftime(buffer,60,"%Y-%M-%d %H:%M:%S", localtime( &rawTime ));
    printf("%s\n", buffer );

    printf("%X , %lu", EOF, sizeof(EOF));
    error_log("test");
    return 0;
}