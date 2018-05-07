#include "common.h"
void insert_sort(int arr[], int len);

int main()
{
    int arr[]={9,3,4,2,6,7,5,1};
    int len = sizeof( arr ) / sizeof(int);

    insert_sort(arr,len);

    for (int i = 0; i < len; i++)
        printf("%d ", arr[i]);
    printf("\n");

    return 0;
}

void insert_sort( int arr[] , int len)
{
    for (int i = 0, j = 0; i < len; i++)
    {
        int temp = arr[i];
        for (j = i - 1; j >= 0; j--)
        {
            if( arr[j] > temp )
                arr[j+1] = arr[j];
            else
                break;
        }
        arr[j+1] = temp;
    }
}
