#include <stdio.h>
#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
int main( int argc, char *argv[] )
{
    int i;

    initscr();
    clear();

    for( i = 0; i < LINES; i++ )
    {
        move( i, i + 1 );
        if( i % 2 == 1 )
            standout();
        addstr( "Hello world" );
        if( i % 2 == 1 )
            standend();
        sleep(1);
        refresh();

    }
    endwin();
}
