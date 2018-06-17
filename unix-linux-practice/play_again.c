#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

#define QUESTION "Need another transaction"
#define TRIES 10
#define SLEEPTIME 1
#define BEEP putchar('\a');

int  get_response( char*, int);
void set_crmode();
void tty_mode( int );
void set_no_delay_mode();
void ctrl_c_handler( int );

int main( int argc, char *argv[] )
{
    int response;
    tty_mode( 0 );
    set_crmode();
    set_no_delay_mode();
    signal( SIGINT, ctrl_c_handler );   // 使用 ctrl_c_handler 函数处理 SIGINT 信号
    signal( SIGQUIT, SIG_IGN);          // 忽略掉 SIGQUIT 信号
    response = get_response( QUESTION, TRIES );
    tty_mode( 1 );
    return response;
}

int get_response( char *question, int maxtries )
{/*{{{*/
    int input;
    printf("%s(y/n)?",question);
    fflush( stdout );
    while( 1 )
    {
        sleep( SLEEPTIME );
        switch( input = getchar() ) 
        {
            case 'y':
            case 'Y':
                return 0;

            case 'n':
            case 'N':
                return 1;

            default:
                printf( "wait %ds ....\n", maxtries );
                maxtries --;
                if( maxtries == 0 )
                    return 2;
        }   
    }
}/*}}}*/

void set_crmode()
{/*{{{*/
    struct termios ttystate;
    tcgetattr( 0, &ttystate );
    ttystate.c_lflag &= ~ICANON; // 关闭缓冲
    // ttystate.c_lflag &= ~ECHO;   // 关闭回显
    // ttystate.c_cc[VMIN] = 1;     // 每次读取一个字符
    tcsetattr( 0, TCSANOW, &ttystate );
}/*}}}*/

void set_no_delay_mode()
{/*{{{*/
    int termflags;
    termflags = fcntl( 0, F_GETFL );
    termflags |= O_NDELAY;
    fcntl( 0, F_SETFL, termflags );
}/*}}}*/

void tty_mode( int how )
{/*{{{*/
    static struct termios original_mode;
    static int            original_flags;
    if( how == 0 )
    {
        tcgetattr( 0, &original_mode );
        original_flags = fcntl( 0, F_GETFL );
    }
    else
    {
        tcsetattr( 0, TCSANOW, &original_mode );
        fcntl( 0, F_SETFL, original_flags );
    }
}/*}}}*/

void ctrl_c_handler( int signum )
{/*{{{*/
    printf("signal SIGINT catched , num : %d ... \n", signum);
    tty_mode( 1 );
    exit( 3 );
}/*}}}*/
