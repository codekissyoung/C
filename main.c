#include "include/common.h"
int main(int argc,const char* argv[]){

	if( !argv[1] ){/*{{{*/
		argv[1] = "default";
	}/*}}}*/

	// 测试缓冲区
	// 1.碰见 \n
	// 2.缓冲区满
	// 3.碰见输出
	if( strcmp("io-cache",argv[1]) == 0 ){
		printf("test io-cache !");
		printf("test io-cache !");
		int input;
		scanf("%d",&input);
		sleep(2);
		return 0;
	}

	// 打印程序的版本
	if( (strcmp("-v",argv[1]) == 0) || (strcmp("--version",argv[1]) == 0) ){/*{{{*/
		const char *glibc_version = gnu_get_libc_version();
		printf("cky version : %.2f \n",0.01);
		printf("glibc version : %s \n",glibc_version);
	}/*}}}*/

	// node 链表操作
	if(strcmp("node",argv[1]) == 0){/*{{{*/
		// 给定一个数字，创建链表存储数据，然后遍历打印出来
		printf("How much number you want to save : ");
		int num;
		scanf("%d",&num);
		
		struct node Head; // 链表的头部
		Head.next = NULL; // 链表的头部
		Head.data = 0;    // 链表头部的数据存0
		struct node *now; // 当前节点

		for ( int i = 0; i < num ; i++) {
			if( Head.next == NULL){
				now = &Head;
			}
			printf("Enter number your want save : ");
			int save;
			scanf("%d",&save);
			struct node *s = init(save);
			now -> next = s;
			now = s;
			printf("you enter %d \n",s -> data );
		}

		// 遍历存进去的链表
		now = &Head;
		while( now != NULL ){
			printf("your data : %d \n",now -> data);
			now = now -> next; // 继续下一个节点
		}
		return 0;
	
	}/*}}}*/

	// 队列
	if (strcmp("queue",argv[1]) == 0) {/*{{{*/
		struct queue q;
		q.head = 1;
		q.tail = 1;
		for ( int i = 1; i <= 5; i++){
			printf("向队列存一个数 : ");
			scanf("%d",&q.data[q.tail]);
			q.tail ++; //队尾向后加1
		}
		
		while( q.head < q.tail ){
			printf("出队列中的值: %d \n",q.data[q.head]);
			q.head++;
		}
		return 0;
	}/*}}}*/

	// 队列
	if (strcmp("queue-qq",argv[1]) == 0) {/*{{{*/
		int q[102] = {0,6,3,1,7,5,8,9,2,4} , head , tail;
		head = 1;
		tail = 10;

		while( head < tail ){
			printf("%d",q[head]);
			head ++;

			q[tail] = q[head];
			head ++; // 队首出队
			tail ++; // 队尾+1
		}
		return 0;
	}/*}}}*/

	// 快速排序
	if(strcmp("quick_sort",argv[1]) == 0){/*{{{*/
		// int test_data[] = {10,1,2,6,8,7,5,9,3,16,15,12,4,14,11,13};
		int test_data[] = {2,1,3,5,4};
		int lenth = sizeof(test_data) / sizeof(int);
		printf("快排演示 %d \n",lenth);
		
		for(int i = 0; i < sizeof(test_data) / sizeof(int); i++){
			printf("%d\t",test_data[i]);
		}
		printf("\n");
		
		sort_in_quick(test_data,0,lenth - 1);
		
		return 0;
	}/*}}}*/

	// 线程互斥
	if(strcmp("thread_mutex",argv[1]) == 0){/*{{{*/
		
	}/*}}}*/

	// 线程退出
	if(strcmp("thread_exit",argv[1]) == 0){/*{{{*/
		pthread_t tid1 , tid2 , tid3;
		void **res;
		pthread_create(&tid1,NULL,first_thread,NULL);
		pthread_create(&tid2,NULL,second_thread,NULL);
		pthread_create(&tid3,NULL,third_thread,NULL);
		pthread_join( tid1 , res ); // 获取进程退出资源
		sleep(10);
	}/*}}}*/

	// thread access
	if(strcmp("thread_access",argv[1]) == 0){/*{{{*/
		pthread_t tid , tid2;

		HS arg;

		int stack = 3;

		arg.heap = (int *)malloc(sizeof(int));

		*(arg.heap) = 2;

		arg.stack = &stack;


		// 打开文件
		
		FILE *file_point = NULL;
		if( (file_point = fopen("text.txt","wb")) == NULL ){
			printf("打开文件失败");
			exit(1);
		}
		
		if( (pthread_create(&tid,NULL,thread_callback,(void *)&arg)) != 0){
			printf("创建线程失败");
			exit(1);
		}

		sleep(10);

		printf("线程返回后，主进程中的数据: %d ,%d \n",(int)*(arg.heap),(int)*(arg.stack));

		// 释放资源
		fclose(file_point);
		return 0;
	}/*}}}*/

	// 线程与进程
	if(strcmp("thread",argv[1]) == 0){/*{{{*/
		// 需要传递给线程的参数
		ARG arg;
		strcpy(arg.arg1,argv[2]);
		arg.arg2 = atoi(argv[3]);
		arg.arg3 = atof(argv[4]);

		pthread_t tid = pthread_self();
		int err = pthread_create(&tid,NULL,print_pro_thread_id,(void *)&arg);

		if(err != 0){
			printf("create thread fail \n");
			exit(1);
		}
		
		sleep(2);
		printf("the main thread pid is %u ,tid is : %u \n",(unsigned int)getpid(),(unsigned int)tid);
		return 0;
	}/*}}}*/
	
	// 从指定的消息队列中读出数据
	if( strcmp("ipc-queue-rcv",argv[1]) == 0){/*{{{*/
		// argv[2] 存在

		int qid = atoi( argv[2] );
		struct msg pmsg;
		int len = msgrcv( qid , &pmsg , BUFSZ , 0 , 0 );
		if( len > 0 ){
			pmsg.msg_buf[len] = '\0';
			printf("reading queue id : %d \n", qid);
			printf("message type : %05ld \n",pmsg.msg_types);
			printf("msssage length : %d \n",len);
			printf("message text : %s \n",pmsg.msg_buf);
		}else if (len == 0 ){
			printf("have no message : %d \n",qid);
		}else{
			perror("msgrcv");
			exit(1);
		}
		system("ipcs -q");
		exit(0);
	}/*}}}*/

	// 进程间的通信 - 消息队列
	if(strcmp("ipc-queue",argv[1]) == 0){/*{{{*/
		key_t key = 113;
		struct msg pmsg; // 消息的结构体变量
		pmsg.msg_types = getpid();
		sprintf(pmsg.msg_buf , "hello! this is : %d \n",getpid());
		int len = strlen(pmsg.msg_buf);

		int qid = msgget( key ,IPC_CREAT | 0666);
		if( qid < 0){
			perror("msgget error!");
			exit(1);
		}else{
			printf("created queue id : %d \n",qid);
			// 向消息队列中发送消息
			if( (msgsnd(qid , &pmsg ,len ,0)) < 0 ){
				printf(" msgsn");
				exit(1);
			}
			printf("successfully send a message to the queue : %d \n",qid);
			system("ipcs -q");
			/*
			if(msgctl(qid , IPC_RMID ,NULL) < 0){
				perror("删除队列失败");
				exit(1);
			}
			*/
			exit(0);
		}
	}/*}}}*/

	// 两个子进程之间的通信
	if(strcmp("brother-pipe",argv[1]) == 0){/*{{{*/
		pro_start();
		int fd[2];
		char buf[PIPE_BUF];
		pid_t pid,pid2;
		int len;

		if( pipe(fd) < 0  ) {
			printf("error pipe");
			exit(1);
		}

		if( ( pid = fork()) < 0 ){
			printf("error ");
			exit(1);
		}else if(pid == 0){
			pro_start();
			sleep(1);
			close(fd[0]);
			write(fd[1],"hello brother!\n", 15);
			printf("子进程\n");
			pro_end();
			exit(0);
		}else{
			if( (pid = fork()) < 0 ){
				printf("error fork!");
				exit(1);
			}else if( pid == 0 ){
				pro_start();
				sleep(2);
				close(fd[1]);
				len = read(fd[0],buf,PIPE_BUF);
				write(STDOUT_FILENO,buf,len);
				printf("子进程2\n");
				pro_end();
				exit(0);
			}
		}
		pro_end();
		exit(0);
	}/*}}}*/

	// 管道
	if(strcmp("pipe",argv[1]) == 0){/*{{{*/
		pro_start();
		int fd[2];
		char buf[PIPE_BUF];
		ssize_t len;
		char str[256];
		if( pipe(fd) < 0){
			printf("pipe error\n");
			exit(1);
		}
		pid_t pid = fork();
		if(pid < 0){
			printf("error fork \n");
			exit(1);
		}else if(pid == 0){
			pro_start();
			close(fd[1]);
			len = read(fd[0],buf,PIPE_BUF);
			write(STDOUT_FILENO,buf,len);
			pro_end();
		}else {
			close(fd[0]);
			write(fd[1],"hello my son! \n",15);
			sleep(10);
			pro_end();
		}
	}/*}}}*/

	// 信号
	if(strcmp("signal",argv[1]) == 0){/*{{{*/
		// 1. 异步的
		// 2. 信号 = 软件中断, 中断就是会打断原来的执行流程，来处理该信号
		// 3. kill -l 查看信号, signal.h 文件定义信号
	}/*}}}*/

	// no-zombie
	if(strcmp("no-zombie",argv[1]) == 0){/*{{{*/
		pid_t pid = fork();

		if(pid < 0){

		}else if(pid == 0){
			pid_t pid = fork();
			if(pid < 0 ){
			}else if(pid == 0){
				printf("child's child!!!\n");
				exit(0);
			}else{
				exit(0);
			}
			exit(0);
		}else{
			if(waitpid(pid,NULL,0) > 0){
				printf("child process exit!!!\n");
			}
			sleep(30);
		}
	}/*}}}*/

	// 僵尸进程的产生
	if(strcmp("zombie",argv[1]) == 0){/*{{{*/
		pid_t pid = fork();
		if(pid < 0){
		}else if(pid == 0){
			printf("the child process %d start \n", getpid());
			sleep(3);
			printf("the child process %d end \n", getpid());
			// 子进程退出了，父进程还在运行，并且没有调用 wait 清理子进程，则子进程就成了zombie
			exit(0);
		}else{
			sleep(30);
			if(wait(NULL) == -1){
				perror("fail to wait");
			}
			printf("the parent process %d end \n", getpid());
			exit(0);
		}
	}/*}}}*/

	// wait
	if(strcmp("wait",argv[1]) == 0){/*{{{*/
		pid_t pid = fork();
		int num,status;
		if(pid < 0){
			perror("fail to fork");
			exit(1);
		}else if(pid == 0){
			/*
			if(execl("./shell" , "first" , NULL) < 0){
				printf("execl fault");
			}
			*/
			printf("the first , exit normally \n");
			sleep(3);
			exit(0);
		}else{
			// waitpid 不阻塞等待进程
			printf("\n----------parent process %d  ----------\n",getpid());

			if(waitpid(pid,NULL,WNOHANG) == 0){
				printf("the child is not available now \n");
			}

			printf("no waiting , parent done \n");

			/*
			if( wait(&status) == -1 ){
				printf("fail to error");
				exit(1);
			}

			printf("阻断了？\n");

			if( WIFEXITED(status) == 1){
				printf("the status of first is : %d \n",WEXITSTATUS(status));
			}
			*/
		}
	}/*}}}*/

	// 多进程操作
	if(strcmp("proc",argv[1]) == 0){/*{{{*/
		pid_t pid = fork();
		if(pid < 0){
			printf("fork 出错");
		}else if(pid == 0){
			printf("-------child porcess %d start ----------\n",getpid());
		    execl("hello","a",NULL);
		    printf("-------child porcess %d end ----------\n",getpid());
		}else {
			printf(" 父进程 ");
		}
	}/*}}}*/

	// 线程操作
	if(strcmp("vfork",argv[1]) == 0){/*{{{*/
		int stack = 1;
		int *heap = (int *) malloc(sizeof(int));
		*heap = 100;
		// 进程中数据
		printf("before vfork : global : %d , stack : %d , *heap : %d \n",global ,stack ,*heap);
		pid_t pid = vfork();
		if(pid < 0){
			printf("vfork error");
		}else if(pid == 0){
			printf("thread process %d start\n",getpid());
			global ++ ;
			stack ++;
			(*heap) ++ ;
			printf("after vfork in thread : global : %d , stack : %d , *heap : %d \n",global ,stack ,*heap);
			printf("thread process %d end\n",getpid());
			exit(0);
		}else {
			printf("in process : global : %d , stack : %d , *heap : %d \n",global ,stack ,*heap);

		}
	}/*}}}*/

	// 移位操作
	if(strcmp("shift",argv[1]) == 0){/*{{{*/
		int a = 12;
		a = a >> 2;
		printf("a : %d \n",a);
	}/*}}}*/

	switch(*argv[1]){
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
			// print_diamond(11);
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

		// 测试time.h
		case 's':/*{{{*/
			{
				time_t cur_time;
				if((cur_time = time(NULL)) == -1){
					perror("time");
					exit(0);
				}
				printf("the current time : %d \n",(int)cur_time);
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
			/*
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
			*/
			break;/*}}}*/

		// 链表
		case 'w':/*{{{*/
			/*
						*/
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
					// printf("1 / 0 is %d \n",1 / 0);;
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
		case '4':/*{{{*/
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
			break;/*}}}*/

	} // end of switch

	if(strcmp("default",argv[1]) == 0){/*{{{*/
		printf("输入 cky 参数 运行特定程序 \n");
	}/*}}}*/

	return 0;
}
















