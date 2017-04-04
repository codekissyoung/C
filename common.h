/* 头文件区 */
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h> /* linux系统调用的头文件 */
#include <unistd.h> /* 进程相关函数头文件 */
#include <stdarg.h>
#include <string.h>
#include <errno.h> /* 错误号申明头文件 */

/* 下面两个头文件定义了相关最大值和最小值的的常量 */
#include <limits.h> /* 整数类型大小限制 */
#include <float.h> /* 浮点类型大小限制 */

/* 专门处理字符的函数*/
#include <ctype.h>
/*全局宏*/
#define MAX 64
#define MSG "I am special"
#define STLEN 14
typedef struct test_struct Test;

/*全局变量申明*/
extern float a1;
extern int b1;
extern float PI;
extern char ** environ;

struct test_struct{
	int array[2];
	char ch;
};

/*全局接口函数*/
extern int max(int a, int b);
extern int swap(int *a,int *b);
extern void f();
extern int add(int a ,int b);
extern int sub(int a ,int b);
extern int mul(int a ,int b);
extern int my_div(int a ,int b);
extern void share();
extern int print_args(int begin, ...);
extern void pointer(void);
extern void pp();
extern void alter(int** p);
extern void print_diamond(int a);
extern void plus(int a);
extern void sum_rows(int ar[][2],int rows);
extern int sum2d(int rows,int cols,int ar[rows][cols]);
