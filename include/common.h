/*        LINUX 系统调用        */
#include <aio.h>            // 异步io
#include <cpio.h>           // 归档值
#include <dirent.h>         // 目录项
#include <dlfcn.h>          // 动态链接
#include <fcntl.h>          // 文件控制
#include <fnmatch.h>        // 文件名匹配类型
#include <glob.h>           // 文件名模式匹配与生成
#include <grp.h>            // 组文件
#include <iconv.h>          // 代码集变换实用程序
#include <langinfo.h>       // 语言信息常量
#include <monetary.h>       // 货币类型与函数
#include <netdb.h>          // 网络数据库操作
#include <nl_types.h>       // 消息类
#include <poll.h>           // 投票函数
#include <pthread.h>        // 线程
#include <pwd.h>            // 口令文件
#include <regex.h>          // 正则表达式
#include <sched.h>          // 执行调度
#include <semaphore.h>      // 信号量
#include <strings.h>        // 字符串操作
#include <tar.h>            // tar 归档值
#include <termios.h>        // 终端I/O
#include <unistd.h>         // 符号常量
#include <wordexp.h>        // 字扩充类型
#include <arpa/inet.h>      // 因特网定义
#include <net/if.h>             // 套接字本地接口
#include <netinet/in.h>     // 因特网地址族
#include <netinet/tcp.h>    // 传输控制协议定义
#include <sys/mman.h>       // 存储管理申明
#include <sys/select.h>     // select 函数
#include <sys/types.h>      // 基本系统数据类型
#include <sys/stat.h>       // 文件状态
#include <sys/statvfs.h>    // 文件系统信息
#include <sys/times.h>      // 进程时间
#include <sys/un.h>         // unix 域 套接字定义
#include <sys/utsname.h>    // 系统名
#include <sys/wait.h>       // 进程控制
#include <fmtmsg.h>         // 消息显示结构
#include <ftw.h>            // 文件树漫游
#include <libgen.h>         // 路径名管理函数
#include <search.h>         // 搜索表
#include <syslog.h>         // 系统出错日志记录
#include <utmpx.h>          // 用户账户数据库
#include <sys/sem.h>            // XSI 信号量
#include <sys/time.h>           // 时间类型
#include <sys/uio.h>        // 矢量I/O操作
#include <sys/resource.h>   // 资源操作
#include <sys/msg.h>        // XSI 消息队列
#include <sys/ipc.h>        // IPC
#include <mqueue.h>         // 消息队列
#include <spawn.h>          // 实时spawn接口

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h> /* 进程相关函数头文件 */
#include <stdarg.h>
#include <string.h>
#include <errno.h> /* 错误号申明头文件 */
#include <time.h>
#include <math.h> /* 数学库 */
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

/* 新类型定义 */
typedef int (*fp1)(int,int);  // 新增一种函数类型指针

/*全局变量申明*/
extern float a1;
extern int b1;
extern float PI;
extern char ** environ;
extern int global;

/*全局接口函数*/
extern int swap(int *a,int *b);
extern void f();
extern int max(int,int);
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
extern void printBook(struct Books *);
// 设计一个调用回调函数的函数
extern int call_func(fp1);

// 快速排序
extern void sort_in_quick(int arr[] , int s, int e);
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

extern int pthread_create(
	pthread_t* restrict tidp,
	const pthread_attr_t* restrict attr ,
	void *(*start_rtn)(void *),
	void *restrict arg);

extern void *thread_callback(void *arg);

// 初始化链表
extern struct node* init(int num);
// 打印队列
extern void show_queue(const struct queue *q);
// 打印栈
extern void show_stack(const struct stack *s);
