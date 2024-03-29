#ifndef INC_2021_FUNC_H
#define INC_2021_FUNC_H

//void error_log(char *);
void echo(int);
void echostr(char*);

void quickSort(int, int[]);
void swap(int*, int*);

void quickSortDemo();
void deadLoopDemo();

// 返回一个表示当前时间的格式化字符串
char* date();

#define SQUARE(x) ((x) * (x))
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define ABS(x) (((x)>=0)?(x):-(x))
#define IS_EVEN(n) ((n)%2==0)
#define TOUPPER(c) ('a'<=(c) && (c)<='z' ? (c)-'a'+'A' : (c))
#define TOLOWER(c) ('A'<=(c) && (c)<='Z' ? (c)+'a'-'A' : (c))
#define PRINT_INT(n) printf(#n " : %ld\n", n)
#define TEST(con,...) ((con)?printf("pass test: %s\n",#con):printf(__VA_ARGS__))
#define FUNC_CALLED() printf("%s called\n", __FUNCTION__);
#define FUNC_RETURN() printf("%s returned\n", __FUNCTION__);
#define error_log(x) printf("[%s] %s:%d " "%s" "\n", date(), __FILE__, __LINE__, x);

#endif
