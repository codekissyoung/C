#include "common.h"

void show_info(struct utmp *a);
void oops(char*, char*);

int main(int ac, char *av[])
{
    struct utmp     record;
    int             utmpfd;
    int             len = sizeof(record);

    utmpfd = open(UTMP_FILE, O_RDONLY);
    if (utmpfd == -1)
    {
        perror(UTMP_FILE);
        exit(1);
    }
    while(read(utmpfd, &record, len) == len)
        show_info(&record);

    close(utmpfd);


    // cp 代码
    int in_fd;
    int out_fd;
    int n_chars;
    char buf[4096];

    in_fd = open( av[1], O_RDONLY );

    out_fd = open( av[2], O_WRONLY );

    while( (n_chars = read(in_fd, buf, 4096)) > 0 )
    {
        if( write(out_fd, buf, n_chars) != n_chars )
            oops("写入出错",av[2]);
    }
    if( n_chars == -1 )
        oops("读取出错", av[2]);

    close(in_fd);
    close(out_fd);

    return 0;
}

void show_info(struct utmp *a)
{
    printf("ut_name: %s,ut_len: %s,ut_time: %d\n",a->ut_name,a->ut_line,a->ut_time);
}

void oops( char *s1, char *s2 )
{
    fprintf(stderr,"Error:%s\n",s1);
    perror(s2);
    exit(1);
}

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
