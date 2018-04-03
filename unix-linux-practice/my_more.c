#include <stdio.h>
#define PAGELINE 10
#define LINELEN 512

int do_more( FILE* );
int see_more( FILE* );

int main ( int argc, char *argv[] )
{
    if( argc == 1 )
    {
        do_more( stdin );
    }
    else
    {
        FILE *fp;
        if( ( fp = fopen( argv[1], "r" ) ) != NULL )
        {
            int ret = do_more( fp );
            fclose( fp );
            return ret;
        }
        else
        {
            return 1;
        }
    }
}

int do_more( FILE *fp )
{
    char    line[LINELEN];
    int     num_of_lines = 0;
    int     reply;
    FILE    *fp_tty;

    if ( ( fp_tty = fopen( "/dev/tty","r")) == NULL )
        return 1;

    while( fgets( line, LINELEN, fp ) )
    {
        if( fputs( line, stdout ) == EOF )
            return 1;
        num_of_lines++;

        if( num_of_lines == PAGELINE )
        {
            printf("more?\n");
            reply = see_more( fp_tty );
            if( reply == 0 )
                break;
            num_of_lines = num_of_lines - reply;
        }
    }
    return 0;
}

int see_more( FILE* cmd )
{
    int c;
    while( (c = getc( cmd )) != EOF )
    {
        if( c == 'q' )
            return 0;
        if( c == ' ' )
            return PAGELINE;
        if( c == '\n')
            return 1;
    }
    return 0;
}
