#include "common.h"
#define BUFSIZE 16
#define NAME "codekissyoung"
struct Books
{
    char title[50];
    char author[50];
    char subject[100];
    int book_id;
};

typedef struct
{
    int a;
    char b;
    double c;
} Simple;

struct NODE
{
    char string[100];
    struct NODE *next_node;
};

struct B;

struct A
{
    struct B *pointer;
    char name[100];
};

struct B
{
    struct A *pointer;
    int age;
};

struct bit_field
{
    int a:4;  //占用4个二进制位;
    int  :0;  //空位域,自动置0;
    int b:4;  //占用4个二进制位,从下一个存储单元开始存放;
    int c:4;  //占用4个二进制位;
    int d:5;  //占用5个二进制位,剩余的4个bit不够存储4个bit的数据,从下一个存储单元开始存放;
    int  :0;  //空位域,自动置0;
    int e:4;  //占用4个二进制位,从这个存储单元开始存放;
};

union Data
{
    int i;
    float f;
    char str[20];
};

void show_info(struct utmp *a);
void oops(char*, char*);
void test_static();

int main(int ac, char *av[])
{
    int i;
    for(i = 0; i < 60; i++)
    {
        sleep(1);
    }
    return 0;

    int n = 10;
    n++;
    n--;
    n += 100;

    int             len = sizeof(struct utmp);
    char            utmpbuf[4*len];
    const char *USERNAME = "codekissyoung";
    struct Books book1 = {
        "《钢铁是如何炼成的》",
        "佚名",
        "保尔柯察金",
        893112
    };
    Simple s1 = {
        329312,
        'b',
        1892.388
    };
    printf("%f\n",s1.c);
    struct NODE node1 = {
        "helloworld",
        NULL
    };
    struct A user_name = {
        NULL,
        "codekissyoung"
    };
    struct B user_age = {
        NULL,
        21
    };
    user_name.pointer = &user_age;
    user_age.pointer = &user_name;

    struct bit_field bt1;
    bt1.a = 3;
    bt1.b = 4;
    bt1.c = 5;
    bt1.d = 6;
    bt1.e = 2;

    union Data d1;
    strcpy( d1.str, "codekissyoung" );

    int utmpfd = open(UTMP_FILE, O_RDONLY);

    DIR *dir_ptr;
    if( (dir_ptr = opendir( "lib" )) == NULL )
        oops("opendir","Error");

    struct dirent *direntp;
    while( (direntp = readdir( dir_ptr )) != NULL )
    {
        printf("%s\n",direntp -> d_name );
    }

    struct stat infobuf;
    if( stat("main.c",&infobuf) == -1 )
        oops("stat","Error");
    else
        printf("infobuf.size : %ld\n", infobuf.st_size);

    closedir(dir_ptr);

    read(utmpfd, utmpbuf, 4 * len);
    struct utmp *recive  = (struct utmp *)&utmpbuf[0];
    close(utmpfd);
    test_static();
    test_static();
    test_static();
    // cp 代码
    /*
    int in_fd;
    int out_fd;
    int n_chars;
    char buf[BUFSIZE];

    in_fd = open( av[1], O_RDONLY );

    out_fd = creat( av[2], 0644 );
    if( out_fd == -1 )
        oops("outfd error", av[2]);

    while( (n_chars = read(in_fd, buf, BUFSIZE)) > 0 )
    {
        if( write(out_fd, buf, n_chars) != n_chars )
            oops("写入出错",av[2]);
    }
    if( n_chars == -1 )
        oops("读取出错", av[2]);

    close(in_fd);
    close(out_fd);
    */
    return 0;
}

void test_static()
{
    static int i;
    i++;
    printf("i : %d\n",i);
}

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
