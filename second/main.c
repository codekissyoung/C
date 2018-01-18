#include <stdio.h>
#define MAXLINE 1000
int my_getline( char line[], int maxline );
void copy( char to[], char from[] );

int main()
{/*{{{*/
    int c, i, nwhite, nother;
    int ndigit[10];

    nwhite = nother = 0;
    for( i = 0; i < 10; i++ )
    {
        ndigit[i] = 0;
    }

    while( ( c = getchar() ) != EOF )
    {
        if( c >= '0' && c <= '9')
        {
            ndigit[ c - '0' ]++;
        }
        else if( c == ' ' || c == '\t' || c == '\n' )
        {
            nwhite++;
        }
        else
        {
            nother++;
        }
    }

    printf("digits = ");
    for( i = 0; i < 10; i++ )
    {
        printf(" %d ", ndigit[i] );
    }
    printf(",white space : %d , other char : %d \n", nwhite, nother);


    char article[MAXLINE];
    int len;
    int j = 1;
    while ( ( len = my_getline( article, MAXLINE ) ) != 0 )
    {
        printf( "line %d : %s", j, article );
        j++;
    }

    return 0;
}/*}}}*/

int my_getline( char s[], int lim )
{/*{{{*/
    int c, i;
    for ( i = 0; i < lim - 1 && ( c = getchar() ) != EOF; ++i )
    {
        s[i] = c;
        if( c == '\n' )
        {
            s[i+1] = '\0';
            break;
        }
    }
    return i;
}/*}}}*/

