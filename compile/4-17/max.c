#include "common.h"
void* max( void* arr[] ,int len, cmp func )
{
    void* max;
    max = arr[0];

    for( int i = 1; i < len; i++ )
    {
        if( ( * func )( max, arr[i] ) )
        {
            max = arr[i];
        }
    }
    return max;
}
