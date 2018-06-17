#include <stdio.h>
#include <termios.h>
#define oops(s,x) { perror(s); exit(x); }
int main( int argc, char *argv[] )
{
    struct termios info;

    tcgetattr( 0, &info );

    if( argv[1][0] == 'y' )
        info.c_lflag |= ECHO;
    else
        info.c_lflag &= ~ECHO;
    
    tcsetattr( 0,TCSANOW, &info );

    return 0;
}
