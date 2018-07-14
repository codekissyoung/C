#ifndef __COMMON_H_
#define __COMMON_H_

#include <stdio.h>
#include <utmp.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>

#define BUFSIZE 16
#define NAME "codekissyoung"
#define min(m,n) ((m) < (n) ? (m) : (n))
#define max(m,n) ((m) > (n) ? (m) : (n))

#ifdef __GNUC__
    #define NORETURN __attribute__ ((__noreturn__))
#else
    #define NORETURN
#endif

typedef enum { FALSE, TRUE } Boolen;
typedef unsigned char BYTE;

struct Books
{/*{{{*/
    char title[50];
    char author[50];
    char subject[100];
    int book_id;
};/*}}}*/

typedef struct Simple
{/*{{{*/
    int a;
    char b;
    double c;
} Simple;/*}}}*/

struct NODE
{/*{{{*/
    char string[100];
    struct NODE *next_node;
};/*}}}*/

struct B;
struct A
{/*{{{*/
    struct B *pointer;
    char name[100];
};/*}}}*/

struct B
{/*{{{*/
    struct A *pointer;
    int age;
};/*}}}*/

struct bit_field
{/*{{{*/
    int a:4;  //占用4个二进制位;
    int  :0;  //空位域,自动置0;
    int b:4;  //占用4个二进制位,从下一个存储单元开始存放;
    int c:4;  //占用4个二进制位;
    int d:5;  //占用5个二进制位,剩余的4个bit不够存储4个bit的数据,从下一个存储单元开始存放;
    int  :0;  //空位域,自动置0;
    int e:4;  //占用4个二进制位,从这个存储单元开始存放;
};/*}}}*/

union Data
{/*{{{*/
    int i;
    float f;
    char str[20];
};/*}}}*/

int MAX( const int a, const int b );
void minprintf( char *fmt, ... );
void filecopy( FILE *ifp, FILE *ofp );
void show_info( struct utmp *a );
void oops( char*, char* );
void test_static();
void insert_sort( int arr[], int len );
void merge_sort( int a[], int first, int last );
void errMsg( const char *format, ... );
void errExit( const char *format, ... ) NORETURN;
void err_exit( const char *format, ... ) NORETURN;
void errExitEN(int errnum, const char *format, ...) NORETURN;
void fatal(const char *format, ...) NORETURN;
void usageErr(const char *format, ...) NORETURN;
void cmdLineErr(const char *format, ...) NORETURN;
int getInt( const char *arg, int flags, const char *name );
int getLong( const char *arg, int flags, const char *name );

#endif
