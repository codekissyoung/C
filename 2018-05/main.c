#include "common.h"

int main(int argc, char *argv[])
{
    FILE *fp;
    char *prog = argv[0];

    if( argc == 1 )
    {
        filecopy(stdin, stdout);
    }
    else
    {
        while( --argc > 0 )
        {
            if( NULL == (fp = fopen( *++argv, "r")) )
            {
                fprintf( stderr, "%s can not open file : %s\n", prog, *argv );
                exit(1);
            }
            else
            {
                filecopy( fp, stdout );
                fclose( fp );
            }
        }
    }

    if( ferror(stdout) )
    {
        fprintf( stderr, "%s : error writing stdout \n", prog );
        exit(2);
    }

    minprintf("Hello world %d, %f,%s\n", 10, 893.2233423, "Codekissyoung");
    return 0;
}
