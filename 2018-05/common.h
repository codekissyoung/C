#include <stdio.h>
#include <utmp.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

struct Books
{
    char title[50];
    char author[50];
    char subject[100];
    int book_id;
};

typedef struct{
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
void insert_sort(int arr[], int len);
void merge_sort(int a[], int first, int last);
