#include "../include/common.h"

void pro_start(){/*{{{*/
	printf("-----------------process %d start -------------------- \n",getpid());
	self_info();
}/*}}}*/

void pro_end(){/*{{{*/
	printf("-----------------process %d end ---------------------- \n\n\n",getpid());
}/*}}}*/

int max(int a, int b){/*{{{*/
	if(a > b)
		return a;
	else
		return b;
}/*}}}*/

int swap(int *a,int *b){/*{{{*/
	int temp;
	temp = *a;
	*a = *b;
	*b = temp;
	return *a + *b;
}/*}}}*/

void f(){/*{{{*/
	printf("b1 in f() : %d \n", b1);
}/*}}}*/

// 不定参数
int print_args(int begin, ...){/*{{{*/
	va_list ap;
	char *p;
	int n = 0;
	va_start(ap,begin);

	p = va_arg(ap, char*);
	printf("arg %d : %s\n",n,p);
	while(p != NULL){
		n++;
		printf("arg %d : %s\n",n,p);
		p = va_arg(ap, char*);
	}
	va_end(ap);
	return n;
}/*}}}*/

// 指针
void pointer(void){/*{{{*/
	int a[5] = {1,2,3,4,5};
	int (*p)[5];
	int *ptr;
	p = &a;
	ptr = (int *)(p + 1);
	printf("the result is : %d \n",*(ptr - 1));
}/*}}}*/

// 指针的指针
void pp(){/*{{{*/
	int a = 100;
	int *p = &a;
	int **q = &p;
	printf("*p : %d \n",*p);
	printf("p : %p \n",p);
	printf("*q : %p \n",*q);
}/*}}}*/

// 改变指针的指向
void alter(int** p){/*{{{*/
	int *q;
	q = (int *)malloc(sizeof(int));
	*q = 100;
	*p = q;
}/*}}}*/

void _print_space(int a){/*{{{*/
	for(int i = 0;i < a;i++){
		printf(" ");
	}
}/*}}}*/

void _print_star(int a){/*{{{*/
	for(int i = 0;i < a;i++){
		printf("*");
	}
}/*}}}*/

// 打印一个菱形
void print_diamond(int a){/*{{{*/
	printf("菱形长度: %d \n",a);
	int i;
	for(i = 0;i < a;i++){
		_print_space(a - i);
		_print_star((2 * i) - 1);
		printf("\n");
	}

	_print_star(2 * a - 1) ;
	printf("\n");
	for(i = a - 1;i > 0;i--){
		_print_space(a - i);
		_print_star((2 * i) - 1);
		printf("\n");
	}
}/*}}}*/

// 乘法表
void plus(int a){/*{{{*/
	for(int row = 1; row <= a;row++){
		for(int col = 1;col <= row;col ++){
			printf("%d x %d = %d\t",col,row,col * row);
		}
		printf("\n");
	}
}/*}}}*/

// 处理二维数组的函数
void sum_rows(int ar[][2],int rows){/*{{{*/
	for(int i = 0;i < rows;i++){
		for(int j = 0;j < 2;j++){
			printf("ar[%d][%d] :%d \t",i,j,ar[i][j]);
		}
		printf("\n");
	}
}/*}}}*/

// 处理变长数组的函数
int sum2d(int rows,int cols,int ar[rows][cols]){/*{{{*/
    int r;
    int c;
    int tot = 0;
    for(r = 0;r < rows;r++){
        for(c = 0;c < cols;c++){
            tot += ar[r][c];
        }
    }
    return tot;
}/*}}}*/

// 习题11.5
char *pr(char *str){/*{{{*/
	char *pc;
	pc = str;
	while(*pc){
		printf("%p : ",pc);
		putchar(*pc++);
		printf("\n");
	}
	printf("pc : %p \n",pc);

	do{
		printf("%p : ",--pc);
		putchar(*pc);
		printf("\n");
	}while(pc - str);

	return pc;
}/*}}}*/

// 快速排序的划分函数
// arr 该数组
// low 低下标
// high 高下标
void divide(int *arr,int low,int high){/*{{{*/
	if(low >= high){
		return ;
	}

	// 需要交换的情况
	int base      = arr[low]; // 找基准点
	int low_mark  = low; // 低下标
	int high_mark = high; // 高下标
	int temp;
	
	printf("b:");
	for(int i = low;i <= high;i++){
		printf("%d\t",arr[i]);
	}
	printf("\n");

	while(low_mark != high_mark && low_mark < high_mark){
		while(arr[high_mark] >= base && low_mark < high_mark){
			high_mark--;
		}
		while(arr[low_mark] <= base && low_mark < high_mark){
			low_mark++;
		}
		swap(&arr[low_mark],&arr[high_mark]);
	}
	swap(&arr[low],&arr[high_mark]);
	printf("middle index : %d , base : %d\n",high_mark,base);
	printf("a:");
	for(int i = low;i <= high;i++){
		printf("%d\t",arr[i]);
	}
	printf("\n\n");

	divide(arr, low , high_mark - 1);
	divide(arr, high_mark + 1, high);
}/*}}}*/

void show_arr(int arr[],int num){/*{{{*/
	for(int i = 0;i < num;i++){
		printf("%d\t",arr[i]);
	}
	printf("\n");
}/*}}}*/

// 打印队列
void show_queue(const struct queue *q){/*{{{*/
	for(int i = q->head;i < q->tail;i++){
		printf("%d\t",q->data[i]);
	}
}/*}}}*/

// 打印栈
void show_stack(const struct stack *s){/*{{{*/
	for(int i = 0;i < s->top;i++){
		printf("%d\t",s->data[i]);
	}
}/*}}}*/

// 玩家的一次出牌过程(队列数据添加到栈)
void card_out(struct queue *q,struct stack *s){/*{{{*/
	// 出牌
	s->data[s->top] = q->data[q->head];
	s->top++;
	q->head++;
	card_eat(q,s);
}/*}}}*/

// 判断吃牌
void card_eat(struct queue *q,struct stack *s){/*{{{*/
	int num = s->data[s->top-1]; // 要比较的数
	int mark = s->top - 2; // 比较到哪一位才相等

	// 判断是否吃牌
	for(;0 <= mark;mark--){
		if(s->data[mark] == num ){
			// printf(" (吃到:%d位) ",mark);
			// 吃牌
			for(;s->top > mark;s->top--){
				q->data[q->tail] = s->data[s->top-1];
				q->tail++;
			}
			break;
		}
	}
}/*}}}*/

// 测试下gdb
int factorial(int n){/*{{{*/
	int i = 1;
	int sum = 0;
	for(;i<n;i++){
		sum = sum + i;
	}
	return sum;
}/*}}}*/

// 进程中止处理函数
void when_exit(void){/*{{{*/
	printf("\n进程 %d 退出了\n",getpid());
}/*}}}*/

void self_info(){/*{{{*/
	printf("PID: %d  PPID:%d  UID: %d  EUID:%d  GID: %d  EGID: %d\n",getpid(),getppid(),getuid(),geteuid(),getgid(),getegid());
}/*}}}*/

// 打印进程 ID 和 线程ID
void* print_pro_thread_id(void * arg){/*{{{*/
	pid_t pid = getpid();
	pthread_t tid = pthread_self();
	printf("pid : %u , thread id : %u \n",(unsigned int)pid, (unsigned  int)tid);
	
	ARG* p = (ARG*) arg;
	printf("the args : %s , %d ,%f \n",p->arg1 , p->arg2 , p->arg3 );
	return NULL;
}/*}}}*/









