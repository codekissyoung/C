#include <stdio.h>
#define MAXLINE 1000

int my_getline( char line[], int maxline );
void copy( char to[], char from[] );

int main( int argc, char *argv[] )
{
    int len;
    int max;
    char line[MAXLINE];
    char longest[MAXLINE];

    max = 0;

    while( (len = my_getline(line,MAXLINE)) > 0 )
    {
        if( len > max )
        {
            max = len;
            copy( longest, line );
        }
    }
    if( max > 0 )
    {
        printf( "%s", longest );
    }
    return 0;
}

// 读取一行到line中，并返回长度
int my_getline( char line[], int maxline )
{
    char c;
    int i;
    for( i = 0; i < maxline - 1 && ( ( c = getchar() ) != EOF && c != '\n'); ++i )
    {
        line[i] = c;
    }
    if( c == '\n' )
    {
        line[i] = c;
        ++i;
    }
    line[i] = '\0';
    return i;
}

// 将 from 复制到 to
void copy( char to[], char from[] )
{
    int i;
    i = 0;
    while( ( to[i] = from[i] ) != '\0')
        ++i;
}
