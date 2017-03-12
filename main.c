#include <stdio.h>
#include "func.h"
#include "static.h" // 静态库
#include "share.h" // 动态库

int main(int argv,char* argc[]){
	int test = 10;
	int t2 = 20;
	int t3 = test + t2;
	printf("t3 : %d \n",t3);
	// int *a;
	// *a = test;
	// printf("%s \n",a);
	printf("%d \n",max(4,6)); // 调用函数
	printf("argv is %d \n",argv); // 程序输入参数的个数
	for(int i = 0;i < argv;i++){
		printf("%d\n",i);
		printf("argc[%d] is %s \n",i,argc[i]);// 依次打印输入的参数
	}
	// 测试动态库
	share();
	// 测试静态库
	printf("静态库add(3,5)是%d \n",add(3,5));

	goto end;

	// 测试程序运行时间的优化　gcc -O2
	int i, j, x;
	x = 0;
	for(i = 0; i < 100000; i++) {
		// printf("%d \n",i);
		for(j = i; j > 0; j--) {
			x += j;
		}
	}

	end:
		printf("the end \n");
	return 0;
}
