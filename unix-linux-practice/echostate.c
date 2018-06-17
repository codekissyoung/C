#include <stdio.h>
#include <termios.h>
int main( int argc, char *argv[] )
{
    struct termios info;
    
    tcgetattr( 0, &info );

    if( info.c_lflag & ECHO )
        printf("回显开了");
    else
        printf("回显关闭");
    return 0;
}
