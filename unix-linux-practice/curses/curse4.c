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
        move( i, i );
        addstr( "Hello world" );
        refresh();
        
        sleep(1);

        move( i, i );
        addstr( "           " );
        refresh();
    }
    endwin();
}
