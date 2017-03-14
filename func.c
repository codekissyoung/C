#include "common.h"
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
	a1 = 23.29;
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
