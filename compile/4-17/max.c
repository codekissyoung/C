#include "common.h"
void* max( void* arr[] ,int len, cmp func )
{
    int i;
    void* tmp;
    tmp = arr[0];

    for( i = 1; i < len; i++ )
    {
        if( ( * func )( tmp, arr[i] ) )
        {
            tmp = arr[i];
        }
    }
    return tmp;
}
