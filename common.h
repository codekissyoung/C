/* 头文件区 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h> /* linux系统调用的头文件 */
#include <unistd.h> /* 进程相关函数头文件 */
#include <stdarg.h>
#include <string.h>
#include <errno.h> /* 错误号申明头文件 */
/*全局宏*/
#define MAX 64
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
