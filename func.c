#include "common.h"
float a1 = 23.29; // a1 变量定义
int max(int a, int b){
	if(a > b)
		return a;
	else
		return b;
}

int swap(int *a,int *b){
	int temp;
	temp = *a;
	*a = *b;
	*b = temp;
	return *a + *b;
}

void f(){
	printf("b1 in f() : %d \n", b1);
}

// 不定参数
int print_args(int begin, ...){
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
}

// 指针
void pointer(void){
	int a[5] = {1,2,3,4,5};
	int (*p)[5];
	int *ptr;
	p = &a;
	ptr = (int *)(p + 1);
	printf("the result is : %d \n",*(ptr - 1));
}

// 指针的指针
void pp(){
	int a = 100;
	int *p = &a;
	int **q = &p;
	printf("*p : %d \n",*p);
	printf("p : %p \n",p);
	printf("*q : %p \n",*q);
}

// 改变指针的指向
void alter(int** p){
	int *q;
	q = (int *)malloc(sizeof(int));
	*q = 100;
	*p = q;
}

void _print_space(int a){
	for(int i = 0;i < a;i++){
		printf(" ");
	}
}
void _print_star(int a){
	for(int i = 0;i < a;i++){
		printf("*");
	}
}

// 打印一个菱形
void print_diamond(int a){
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
}

// 乘法表
void plus(int a){
	for(int row = 1; row <= a;row++){
		for(int col = 1;col <= row;col ++){
			printf("%d x %d = %d\t",col,row,col * row);
		}
		printf("\n");
	}
}

// 处理二维数组的函数
void sum_rows(int ar[][2],int rows){
	for(int i = 0;i < rows;i++){
		for(int j = 0;j < 2;j++){
			printf("ar[%d][%d] :%d \t",i,j,ar[i][j]);
		}
		printf("\n");
	}
}

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
char *pr(char *str){
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
}

