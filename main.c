#include <stdio.h>
#include "func.h"
#include "static.h" // 静态库
#include "share.h" // 动态库

int main(int argv,char* argc[]){
	printf("%d \n",max(4,6)); // 调用函数
	printf("argv is %d \n",argv); // 程序输入参数的个数
	for(int i = 0;i < argv;i++){
		printf("argc[%d] is %s \n",i,argc[i]);// 依次打印输入的参数
	}
	// 测试动态库
	share();
	// 测试静态库
	printf("静态库add(3,5)是%d",add(3,5));

	// 测试程序运行时间的优化　gcc -O2
	int i, j, x;
	x = 0;
	for(i = 0; i < 100000; i++) {
		for(j = i; j > 0; j--) {
			x += j;
		}
	}
	return 0;
}
