#include "common.h"
void insert_sort(int arr[], int len);
void merge_sort(int a[], int first, int last);

int main()
{
    int arr[]={9,3,4,2,6,7,5,1,78,87,31,24,55,43,11,22,33,44,11,11,11,22,33,443,112,90};
    int len = sizeof( arr ) / sizeof(int);

    // insert_sort(arr,len);
    merge_sort( arr, 0, len - 1);

    for (int i = 0; i < len; i++)
        printf("%d ", arr[i]);
    printf("\n");

    return 0;
}

void merge_sort(int a[], int first, int last)
{
    if( first < last )
    {
        int mid = (first + last) / 2;

        merge_sort(a, first, mid);
        merge_sort(a, mid + 1, last);

        int temp[last-first+1]; // 临时数组(c的可变长数组特性)

        int left  = first;
        int right = mid + 1;
        int k = 0;

        while(left <= mid && right <= last)
        {
            if( a[left] < a[right] )
                temp[k++] = a[left++];
            else
                temp[k++] = a[right++];
        }

        while(left <= mid)
            temp[k++] = a[left++];

        while(right <= last)
            temp[k++] = a[right++];

        for(int i = first, k = 0; i <= last; i++, k++)
            a[i] = temp[k];
    }
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
