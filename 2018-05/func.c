#include "common.h"
void test_static()
{/*{{{*/
    static int i;
    i++;
    printf("i : %d\n",i);
}/*}}}*/

void show_info(struct utmp *a)
{/*{{{*/
    printf("ut_name: %s,ut_len: %s,ut_time: %d\n",a->ut_name,a->ut_line,a->ut_time);
}/*}}}*/

void oops( char *s1, char *s2 )
{/*{{{*/
    fprintf(stderr,"Error:%s\n",s1);
    perror(s2);
    exit(1);
}/*}}}*/

// 归并排序
void merge_sort(int a[], int first, int last)
{/*{{{*/
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
}/*}}}*/

// 插入排序
void insert_sort( int arr[] , int len)
{/*{{{*/
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
}/*}}}*/
