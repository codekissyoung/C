#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h> /* 进程相关函数头文件 */
#include <fcntl.h> /* linux系统调用的头文件 */
#include <stdarg.h>
#include <string.h>
#include <errno.h> /* 错误号申明头文件 */
#include <time.h>
#include <pthread.h> /* 线程 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/msg.h> /*消息队列*/
#include <sys/ipc.h> /*消息队列*/
#include <limits.h> /* 整数类型大小限制 */ /* 下面两个头文件定义了相关最大值和最小值的的常量 */
#include <float.h> /* 浮点类型大小限制 */
#include <ctype.h> /* 专门处理字符的函数*/
#include <gnu/libc-version.h> // gnu glibc version
#include "thread.h"
#include "struct.h"
/*全局宏*/
#define MAX 64
#define MSG "I am special"
#define STLEN 14
#define MAX_THREAD 3  // 最大线程数
#define BUFSZ 4096
#define min(m,n) ((m) < (n) ? (m) : (n))
#define max(m,n) ((m) > (n) ? (m) : (n))

/* 调试代码 */
#ifndef ONLY
	#define ONLY 1
	#define DEBUG 1
	#ifdef DEBUG
		#define debug(a, b) printf(#a"\n",b)
	#else
		#define debug(a,b) ;
	#endif
#endif


/*全局变量申明*/
extern float a1;
extern int b1;
extern float PI;
extern char ** environ;
extern int global;

/*全局接口函数*/
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
extern char *pr(char *str);
extern void quick_sort(int arr[],int num);
extern void divide(int *arr,int low,int high);
extern void show_arr(int arr[],int num);
extern void pro_start();
extern void pro_end();
// 测试不定参数
extern void simple_print_int(int i ,...);
// 程序信息
extern void self_info();
// 出牌过程
extern void card_out(struct queue *q,struct stack *s);
extern void card_eat(struct queue *q,struct stack *s);
// 为了避免出现implicit declaration of function vfork ,warning,所以自己加个声明
extern pid_t vfork(void);
extern pid_t wait3(int *statloc,int options,struct rusage *r);
// 测试gdb
extern int factorial(int n);
extern void when_exit(void);
extern void* print_pro_thread_id(void *arg);
extern int pthread_create(pthread_t* restrict tidp,const pthread_attr_t* restrict attr ,void *(*start_rtn)(void *),void *restrict arg);
extern void *thread_callback(void *arg);
// 初始化链表
extern struct node* init(int num);
// 打印队列
extern void show_queue(const struct queue *q);
// 打印栈
extern void show_stack(const struct stack *s);
// 快速排序
extern void sort_in_quick(int arr[] , int s, int e);
