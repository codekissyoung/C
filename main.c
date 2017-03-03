#include <stdio.h>
#include "func.h"

int main(int argv,char* argc[]){
	printf("%d \n",max(4,6)); // 调用函数
	printf("argv is %d \n",argv); // 程序输入参数的个数
	for(int i = 0;i < argv;i++){
		printf("argc[%d] is %s \n",i,argc[i]);　// 依次打印输入的参数
	}
	return 0;
}
