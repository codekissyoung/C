#include <stdio.h>
#include "func.h"

int main(int argv,char* argc[]){
	int c;
	printf("hello cky!\n");
	c = max(4,6);
	printf("%d \n",c);
	printf("argv is %d \n",argv);
	// int i = 0;
	for(int i = 0;i < argv;i++){
		printf("argc[%d] is %s \n",i,argc[i]);
	}
	return 0;
}
