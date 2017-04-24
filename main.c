#include "common.h"
#include "global.c"
int main(int argv,const char* argc[]){

	printf("----------------------------start----------------------------\n");

	/*{{{*/
	int sum = 0;
	int i = 0;
	for(;i < 100;i++){
		sum = sum + i;
	}

	// printf("[1-100] : %d\n",sum);
	// printf("[1-500] : %d\n",factorial(500));

	if(atexit(when_exit)){
		printf("\nfail to set exit handler!\n");
	}
/*}}}*/

	switch(*argc[1]){
		// 排序字符串
		case 'a':/*{{{*/
			{
				printf("随便输入字符串\n");
				char input[5][100];
				char *sort[5];
				int i = 0;
				while(i < 5 && fgets(input[i],100,stdin) != NULL){
					sort[i] = input[i];
					++i;
				}

				// 将指针指向的数组输出
				printf("before sort : \n");
				i = 0;
				while(i < 5){
					printf("sort[%d] : %s", i ,sort[i]);
					i++;
				}

				// 排序
				int j = 0;
				char *temp;
				for(i = 0;i < 5;i++){
					for(j = 0;j < 5 - i -1;j++){
						if(strncmp(sort[j],sort[j + 1],100) > 0 ){
							temp = sort[j];
							sort[j] = sort[j + 1];
							sort[j + 1] = temp;
						}
					}
				}

				// 将指针指向的数组输出
				printf("after sort :\n");
				i = 0;
				while(i < 5){
					printf("sort[%d] : %s", i ,sort[i]);
					i++;
				}
			}
			break;/*}}}*/

		// 指向多维数组的指针
		case 'b':/*{{{*/
			{
				int zippo[3][2] =
				{
					{2,3},
					{4,5},
					{6,7}
				};
				int (*pz)[2]; // 指向一个含有两个int类型值的数组
				pz = zippo;
				printf("zippo : %p , zippo[0]: %p :zippo[0][0] : %d \n",zippo,zippo[0],zippo[0][0]);
				printf("pz : %p , *pz: %p : **pz: %d \n",pz,*pz,**pz);
				printf("pz = %p,pz + 1:%p \n",pz,pz+1);
				printf("pz[0] = %p,pz[0] + 1:%p \n",pz[0],pz[0]+1);
				printf("*pz = %p,*pz + 1:%p \n",*pz,*pz+1);
				printf("**pz = %d,*(*pz + 1):%d \n",**pz,*(*pz+1));
				printf("**(pz + 1) = %d,*(*(pz + 1) + 1):%d \n",**(pz + 1),*(*(pz + 1)+1));
				printf("pz[0][0] = %d,pz[0][1]:%d \n",pz[0][0],pz[0][1]);
				printf("-----------------\n\n");
			}
			break;/*}}}*/

		// 测试EOF
		case 'c':/*{{{*/
			{
				char test_eof;
				while( (test_eof = getchar()) != EOF){
					putchar(test_eof);
				}
			}
			break;/*}}}*/

		// 测试fgets()
		case 'd':/*{{{*/
			{
				char words[STLEN];
				int i;
				while(fgets(words,STLEN,stdin) != NULL && words[0] != '\n'){
					i = 0;
					while(words[i] != '\n' && words[i] != '\0'){
						i++;
					}
					if(words[i] == '\n'){
						words[i] = '\0';
					}else{
						while(getchar() != '\n'){
							continue;
						}
					}
					fputs(words,stdout);
				}
			}
			break;/*}}}*/

		// 测试下字符串数组
		case 'e':/*{{{*/
			{
				const char *pointer_str[5] = {
					"string1 heheh",
					"string2 ,sdfadf",
					"string3 hdhdhdh",
					"xxixixixi",
					"codsdadfssss"
				};
				char array_str[5][40] = {
					"sdfaiisisis",
					"xixixiix sss",
					"hahah",
					"codekissyoung"
				};
				for(int k = 0;k < 5;k++){
					printf("pointer_str[%d] : %p : %s \n",k,pointer_str[k],pointer_str[k]);
				}
				for(int p = 0;p < 5;p++){
					printf("array_str[%d] : %p : %s \n",p,array_str[p],array_str[p]);
				}
			}
			break;/*}}}*/

		// 数组字符串和指针字符串的区别
		case 'f':/*{{{*/
			{
				char ar[] = MSG;
				char *pt = MSG;
				printf("MSG : %p \n",MSG); // 字面量存储的位置
				printf("ar : %p \n",ar); // 数组存储的位置
				printf("pt : %p \n",pt); // 指针指向的位置
				printf("-----------------\n\n");
			}
			break;/*}}}*/

		// 打印其输入的参数
		case 'g':/*{{{*/
			{
				for(int i = 0;i < argv;i++){
					printf("argc[%d] is %s \n",i,argc[i]);// 依次打印输入的参数
				}
			}
			break;/*}}}*/

		// 处理多维数组的函数
		case 'h':/*{{{*/
			{
				int zippo[3][2] =
				{
					{2,3},
					{4,5},
					{6,7}
				};
				sum_rows(zippo,3);
			}
			break;/*}}}*/

		// 变长数组的使用
		case 'i':/*{{{*/
			{
				int zippo[3][2] =
				{
					{2,3},
					{4,5},
					{6,7}
				};
				int total1 = sum2d(3,2,zippo);
				printf("total1 : %d \n", total1);
			}
			break;/*}}}*/

		// 复合字面量
		case 'j':/*{{{*/
			{
				int total1 = sum2d( 2 , 2 , (int [2][2]){{2,3},{4,5}});
				printf("total2: %d \n",total1);
			}
			break;/*}}}*/

		// 使用预定义宏
		case 'k':/*{{{*/
			printf("File :%s\n", __FILE__ );
			printf("Date :%s\n", __DATE__ );
			printf("Time :%s\n", __TIME__ );
			printf("Line :%d\n", __LINE__ );
			printf("ANSI :%d\n", __STDC__ );
			printf("AUTHOR: %s \n---------------\n\n","codekissyoung");
			break;/*}}}*/

		// 进程相关
		case 'l':/*{{{*/
			printf("当前进程ID : %u\n",getpid());
			printf("当前进程父ID : %u\n",getppid());
			printf("当前用户ID : %u\n",getuid());
			printf("当前有效用户ID : %u\n",geteuid());
			printf("当前组ID : %u\n",getgid());
			printf("当前有效组ID : %u\n",getegid());
			printf("-----------------\n\n");
			break;/*}}}*/

		// 环境变量测试
		case 'm':/*{{{*/
			printf("环境变量测试: \n");
			printf("%s \n",environ[0]);
			printf("%s \n",environ[1]);
			printf("HOME : %s \n",getenv("HOME")); // 获取环境变量HOME
			printf("-----------------\n\n");
			break;/*}}}*/

		// 测试输入
		case 'n':/*{{{*/
			{
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
			}
			break;/*}}}*/

		// 测试读取文件
		case 'o':/*{{{*/
			{
				char ch;
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
			}
			break;/*}}}*/

		// 打印一个菱形
		case 'p':/*{{{*/
			print_diamond(11);
			break;/*}}}*/

		// 打印99乘法表
		case 'q':/*{{{*/
			plus(9);
			break;/*}}}*/

		// 不定参数
		case 'r':/*{{{*/
			print_args(-1,"hello","world",NULL);
			print_args(-1,"Olympic","china","Beijing",NULL);
			break;/*}}}*/

		case 's':/*{{{*/
			{
				char *x = pr("Ho Ho Ho!");
				printf("\nx : %p \n", x);
			}
			break;/*}}}*/

		case 't':/*{{{*/
			{
				// struct book library; // 变量

			}
			break;/*}}}*/

		// 快速排序的实现
		case 'u':/*{{{*/
			{
				int arr[] = {30,1,34,53,2,46,5,30,8,9,100,30,28,18,90,78,30,56,84,33,30,20};
				int arr_size = sizeof(arr)/sizeof(arr[0]);
				// show_arr(arr,arr_size);
				divide(arr,0,arr_size -1);
				// printf("\n after quick sort \n");
				// show_arr(arr,arr_size);
			}
			break;/*}}}*/

		// 扑克牌游戏
		case 'v':/*{{{*/
			{
				struct stack desk;// 桌面
				struct queue player1 = {{2,4,1,2,5,6},0,6}; //玩家1
				struct queue player2 = {{3,1,3,5,6,4},0,6}; //玩家2

				// 输出下玩家的手牌
				while(player1.head < player1.tail && player2.head < player2.tail){

					card_out(&player1,&desk);
					printf("玩家1出牌后:\n");
					printf("玩家1: \t");
					show_queue(&player1);
					printf("\n玩家2: \t");
					show_queue(&player2);

					printf("\n桌面:\t");show_stack(&desk);
					printf("\n-----------------------------------------\n");

					card_out(&player2,&desk);

					printf("玩家2出牌后:\n");
					printf("玩家1: \t");
					show_queue(&player1);
					printf("\n玩家2: \t");
					show_queue(&player2);

					printf("\n桌面:\t");show_stack(&desk);
					printf("\n-----------------------------------------\n");
				}

				// 判断胜负
				if(player1.head == player1.tail){
					printf("\n------------------------------玩家2胜利-------------------------\n");
				}else{
					printf("\n------------------------------玩家1胜利-------------------------\n");
				}

			}
			break;/*}}}*/

		// 链表
		case 'w':/*{{{*/
			{
				// 创建链表的第一个元素
				struct node p;
				p.data = 11;
				p.next = NULL;
				struct node *a = init(15);
				printf("p.data : %d\n",p.data);
				printf("a->data : %d\n",a->data);
			}
			break;/*}}}*/

		// 测试下进程相关的内容 fork
		case 'x':/*{{{*/
			{
				int stack = 1;
				int *heap;
				heap = (int *)malloc(sizeof(int));
				*heap = 2;
				pid_t pid;
				pid = fork();

				if( pid < 0){
					printf("\nfail to fork\n");
					exit(1);
				}else if(pid == 0){
					stack ++;
					(*heap)++;
					global++;
					printf("stack : %d , *heap :%d,global:%d\n",stack,*heap,global);
					printf("this is child , pid is : %u\n",getpid());
				}else{
					sleep(2);
					printf("stack : %d , *heap :%d,global:%d\n",stack,*heap,global);
					printf("this is parent , pid is : %u,child-pid is : %u\n",getpid(),pid);
				}
			}
			break;/*}}}*/

		// 测试共享进程 vfork()
		case 'y':/*{{{*/
			{
				pid_t pid;
				int stack = 100;
				int *heap;
				heap = (int *)malloc(sizeof(int));
				*heap = 200;

				pid = vfork();
				if(pid < 0){
					printf("vfork fail!!!!!\n");
					exit(1);
				}else if(pid == 0){
					global ++;
					stack ++;
					(*heap) ++;
					printf("stack : %d ,*heap : %d, global : %d",stack,*heap,global);
					exit(0);
				}else {
					sleep(2); // 保证子进程先运行
					printf("stack : %d ,*heap : %d, global : %d",stack,*heap,global);
				}
			}
			break;/*}}}*/
		
		// 测试exec()
		case 'z':/*{{{*/
				{
					pid_t pid;
					char *argv[] = {"hello"};
					pid = fork();

					if(pid < 0){
						printf("fail to fork \n");
						exit(1);
					}else if(pid == 0){
						execvp("./hello",argv);
					}else{
						printf("parent!");
					}
				}
				break;/*}}}*/

		// 测试system()
		case '1':/*{{{*/
			{
				system("ls -alh");
			}
			break;/*}}}*/

		// 测试wait()
		case '2':/*{{{*/
			{
				pid_t pid;
				int status;
				
				// 创建第一个子进程
				pid = fork();
				if(pid < 0){
					printf("创建进程失败!\n");
					exit(1);
				}else if(pid == 0){
					printf("the first ,exit normally !\n");
					exit(0);
				}else{
					if(wait(&status) == -1){
						perror("fail to wait!!!\n");
						exit(1);
					}
					if(WIFEXITED(status) == 1){
						printf("the status of first is : %d\n",WEXITSTATUS(status));
					}
				}

				// 创建第二个子进程
				pid_t pid2 = fork();
				if(pid2 == 0){
					printf("the second ,exit abnormally!!!!\n");
					printf("1 / 0 is %d \n",1 / 0);;
				}else if(pid2 < 0){
					printf("创建进程错误\n");
					exit(1);
				}else {
					if(wait(&status) == -1){ // 父进程等待子进程退出
						perror("fail to wait!!!\n");
						exit(1);
					}
					if(WIFSIGNALED(status) == 1){
						printf("the teminated signal is : %d\n",WTERMSIG(status));
					}
				}
			}
			break;/*}}}*/

		// 测试僵尸进程
		case '3':/*{{{*/
			{
				pid_t pid;
				pid = fork();

				if(pid == 0){
					printf("i am child process!\n");
					sleep(10);
					printf("child process done!\n");
					exit(0);
				}
				printf("the parent process!\n");
				sleep(30);
				if(wait(NULL) == -1){ // wait child process over
					perror("fail to wait!\n");
					exit(1);
				}
			}
			break;/*}}}*/

		// 测试wait3函数，用于对子进程进行进程统计
		case '4':
			{
				pid_t pid;
				int status;
				struct rusage rusage;

				pid = fork();
				if(pid < 0){
					exit(1);
				}else if(pid == 0){
					printf("the child \n");
					exit(0);
				}else{
					printf("the parent \n");
					if(wait3(&status,0,&rusage) == -1){
						perror("fail to wait!\n");
						exit(1);
					}
					
					int ru_utime = rusage.ru_utime.tv_sec;
					int ru_stime = rusage.ru_stime.tv_sec;
					printf("utime is %d \n",ru_utime);
					printf("stime is %d \n",ru_stime);
					printf("msgsnd is %ld\n",rusage.ru_msgsnd);
					printf("maxrss is %ld\n",rusage.ru_maxrss);
				}
			}
			break;



		default:/*{{{*/
			printf("运行　./cky n (n 为任意数字)\n");
			break;/*}}}*/

	} // end of switch
	printf("\n------------------------------end----------------------------\n");
	return 0;
}
