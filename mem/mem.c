#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define MEM_SIZE (1024*1024)

int main()
{
    char *some_memory;
    some_memory = (char*)malloc( MEM_SIZE );
    if( some_memory != NULL )
    {
        sprintf( some_memory, "Hello World\n" );
        printf( "%s", some_memory );
    }
    exit(0);
}
