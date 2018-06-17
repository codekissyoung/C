#include <stdio.h>
#include <curses.h>
#include <stdlib.h>
#include <unistd.h>

#define LEFTEDGE 10
#define RIGHTEDGE 30
#define ROW 10
#define MSG "Hello world"
#define BLANK "           "



int main( int argc, char *argv[] )
{
    int i;
    int pos;
    int dir = +1;
    initscr();
    clear();

    pos = LEFTEDGE;
    while( 1 )
    {
        move( ROW, pos );
        addstr( MSG );
        refresh();
        
        sleep(1);

        move( ROW, pos );
        addstr( BLANK );
        refresh();
        pos = pos + dir; 

        if( pos >= RIGHTEDGE )
            dir = -1;
        if( pos <= LEFTEDGE )
            dir = +1;     
    }
    endwin();
}
