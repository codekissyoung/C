#include "common.h"
int main(int argv,char* argc[]){

	// 初始化程序各个全局变量值
	a1 = 13;
	b1 = 14;
	PI = 3.14;

	Test var = {{0x12345678,0x98765432},0x30};
	char *cp;
	// Test *tp;
	cp = (char*)&var;
	printf("*cp : %x ,cp : %p\n",*cp,cp);
	printf("*cp : %x ,cp + 1 : %p \n",*(cp+1),cp + 1);

	int* t100; // 申明一个指针
	alter(&t100); // 改变该指针的指向
	printf("*t100 is %d \n",*t100);

	pp();

	pointer();

	print_args(-1,"hello","world",NULL);

	print_args(-1,"Olympic","china","Beijing",NULL);

	char *str = "hello world \n";
	printf("%s \n",str);

	f();
	printf("a1 = %d , b1 = %d \n",a1,b1);

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
