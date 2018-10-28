#include "sort.h"
#include <string.h>

int compare_scores( const void* score_a, const void* score_b )
{
    return *(int*)score_a - *(int*)score_b;
}

int compare_areas( const void *a, const void *b )
{
   rectangle* ra = (rectangle*)a;
   rectangle* rb = (rectangle*)b;

   int area_a = (ra->width * ra->height);
   int area_b = (rb->width * rb->height);
   
   return area_a - area_b;
}

int compare_names( const void* a, const void* b )
{

    return strcmp( *(char**)a, *(char**)b );
}
