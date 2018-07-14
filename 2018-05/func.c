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

const int MAX( const int a, const int b)
{/*{{{*/
    return a > b ? a : b;
}/*}}}*/

void minprintf(char *fmt, ...)
{/*{{{*/
    va_list ap;
    char *p, *sval;
    int ival;
    double dval;

    va_start(ap, fmt);// 将ap指向第一个参数
    for( p = fmt; *p; p++ )
    {
        if( *p != '%' )
        {
            putchar(*p);
            continue;
        }
        else
        {
            p++;
            switch( *p )
            {
                case 'd':
                    ival = va_arg( ap, int );
                    printf("%d",ival);
                    break;
                case 'f':
                    dval = va_arg( ap, double );
                    printf("%f",dval);
                    break;
                case 's':
                    for( sval = va_arg( ap, char* ); *sval; sval++ )
                        putchar( *sval );
                    break;
                default:
                    putchar(*p);
                    break;
            }
        }
    }
    va_end(ap);
}/*}}}*/

void filecopy( FILE *ifp, FILE *ofp )
{/*{{{*/
    int c;
    while( ( c = getc( ifp ) ) != EOF )
        putc( c, ofp );
}/*}}}*/

