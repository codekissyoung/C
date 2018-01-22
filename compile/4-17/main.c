#include "common.h"
#define MAX 3

int main( int argc, char* argv[] )
{
    Book array_1[MAX];
    int *array_2[MAX];

    int i;
    int id;
    int val;
    char name[10];
    Book res1;
    int *res2;

    for( i = 0; i < MAX; i++ )
    {
        printf("Input id of book : ");
        scanf( "%d", &id );

        printf("Input name of book : ");
        scanf( "%s", name );

        if( insert_struct( &array_1[i], id, name )  == -1 )
            exit(1);
    }

    for( i = 0; i < MAX; i++ )
    {
        printf( "Input int\n" );
        scanf( "%d", &val );

        if( insert_int( &array_2[i], val )  == -1 )
            exit(1);
    }

    res1 = (Book)max( (void**)array_1, MAX, cmp_struct );
    res2 = (int *)max( (void**)array_2, MAX, cmp_int );

    printf( "the max of books : %d , %s \n", res1 -> id, res1 -> name );
    printf( "the max of int : %d\n", *res2 );

    return 0;
}

