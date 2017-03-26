#include "common.h"

/* 调试代码 */
#define DEBUG 1
#ifdef DEBUG
#define debug(a, b) printf(#a"\n",b)
#else
#define debug(a,b) ;
#endif

// 全局变量定义
int b1 = 14;
float PI = 3.14;
Test var = {{0x12345678,0x98765432},0x30};

int main(int argv,char* argc[]){
	// 使用预定义宏
	printf("File :%s\n", __FILE__ );
	printf("Date :%s\n", __DATE__ );
	printf("Time :%s\n", __TIME__ );
	printf("Line :%d\n", __LINE__ );
	printf("ANSI :%d\n", __STDC__ );
	printf("AUTHOR: %s \n---------------\n\n","codekissyoung");

	// 进程相关
	printf("当前进程ID : %u\n",getpid());
	printf("当前进程父ID : %u\n",getppid());
	printf("当前用户ID : %u\n",getuid());
	printf("当前有效用户ID : %u\n",geteuid());
	printf("当前组ID : %u\n",getgid());
	printf("当前有效组ID : %u\n",getegid());
	printf("-----------------\n\n");

	printf("环境变量测试: \n");
	printf("%s \n",environ[0]);
	printf("%s \n",environ[1]);
	printf("HOME : %s \n",getenv("HOME")); // 获取环境变量HOME
	printf("-----------------\n\n");

	// 测试输入
	printf("输入字符测试,重复输入的字符，如果读取到q字符，就跳出输入输出\n");
	char ch;
	char left_str[10];

	do{
		ch = getchar();
		if(ch == 'q') break;
		putchar(ch);
	}while (ch != 'q');
	
	if(scanf("%s",left_str)){
		printf("缓冲区读取到的q后面的字符串:%s \n",left_str);
	}

	// 测试读取文件
	FILE* fp = fopen("test.txt","r");
	if(fp){
		while((ch = getc(fp)) != EOF){ // 读取文件内容显示
			putchar(ch);
		}
		fclose(fp); // 关闭文件
	}else{
		printf("打开文件失败\n");
		exit(1);
	}

	// 打印一个菱形
	// print_diamond(11);

	// 打印99乘法表
	// plus(9);

	print_args(-1,"hello","world",NULL);
	print_args(-1,"Olympic","china","Beijing",NULL);

	char *str = "hello world \n";
	printf("%s \n",str);

	printf("argv is %d \n",argv); // 程序输入参数的个数
	for(int i = 0;i < argv;i++){
		printf("argc[%d] is %s \n",i,argc[i]);// 依次打印输入的参数
	}

	goto end;
	printf("测试 goto ");
	end:
		printf("the end \n");
	return 0;
}
