#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "func.h"
#include "static.h" // 静态库
#include "share.h" // 动态库


float PI = 3.14; // 全局变量

int main(int argv,char* argc[]){

	// float PI = 9.12;
	printf("打印全局变量 PI : %f \n",PI);


	printf("argv is %d \n",argv); // 程序输入参数的个数
	for(int i = 0;i < argv;i++){
		printf("argc[%d] is %s \n",i,argc[i]);// 依次打印输入的参数
	}
	printf(" max(4,6) : %d \n",max(4,6)); // 调用函数
	
	share();// 测试动态库
	

	printf("静态库add(3,5)是%d \n",add(3,5)); // 测试静态库

	goto end;
	printf("测试 goto ");


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
