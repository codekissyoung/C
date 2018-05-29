#include "common.h"
#include "lib/ename.inc.c"
#define BUFSIZE 16
#define NAME "codekissyoung"

int main(int ac, char *av[])
{
    printf("File :%s\n", __FILE__ );
    printf("Date :%s\n", __DATE__ );
    printf("Time :%s\n", __TIME__ );
    printf("Line :%d\n", __LINE__ );
    printf("ANSI :%d\n", __STDC__ );

    BYTE b1,b2;
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

/*错误处理*/
static void terminate(Boolen useExit3)
{/*{{{*/
    char *s;
    s = getenv("EF_DUMPCORE");

    if(s != NULL && *s != '\0')
        abort();
    else if (useExit3)
        exit(EXIT_FAILURE);
    else
        _exit(EXIT_FAILURE);
}/*}}}*/

static void outputError(Boolen useErr, int err, Boolen flushStdout, const char *format, va_list ap)
{/*{{{*/
    #define BUF_SIZE 500
    char buf[BUF_SIZE * 3];
    char userMsg[BUF_SIZE];
    char errText[BUF_SIZE];

    vsnprintf(userMsg, BUF_SIZE, format, ap);

    if(useErr)
        snprintf( errText, BUF_SIZE, "[%s %s]", (err>0 && err <= MAX_ENAME) ? ename[err] : "?UNKNOWN?", strerror(err) );
    else
        snprintf( errText, BUF_SIZE, ":" );

    snprintf( buf, BUF_SIZE * 3, "ERROR%s %s\n", errText, userMsg );

    if(flushStdout)
        fflush(stdout);
    fputs(buf,stderr);
    fflush(stderr);
}/*}}}*/

void errMsg(const char *format, ...)
{
    va_list argList;
    int savedErrno;
    savedErrno = errno;
    va_start(argList,format);
    outputError(TRUE,errno,TRUE,format,argList);
    va_end(argList);
    errno = savedErrno;
}


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
