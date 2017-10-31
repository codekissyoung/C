#include "include/common.h"
int main(int argc,const char* argv[])
{
    // 程序版本
    if( 1 == argc || (strcmp("-v",argv[1]) == 0) || (strcmp("--version",argv[1]) == 0) )
    {/*{{{*/
        puts("------------------------- cky程序用法 ----------------------");
        printf("cky version : %.2f\n",0.01);
        printf("glibc version : %s\n" , gnu_get_libc_version() );
        printf("环境变量HOME : %s\n",getenv("HOME"));
        printf("当前进程PID : %d\n",getpid());
        printf("当前进程父PID : %d\n",getppid());
        printf("用户的uid : %d\n",getuid());
        printf("有效用户的uid : %d \n",geteuid());
        printf("当前用户组gid : %d\n",getgid());
        printf("有效用户组gid : %d\n",getegid());

        struct passwd *my_info = getpwuid(getuid());
        if( my_info )   //   是否获得info信息
        {
            printf("My Login Name:%s\n",        my_info -> pw_name);
            printf("My Password :%s\n",         my_info -> pw_passwd);
            printf("My User ID :%d\n",          my_info -> pw_uid);
            printf("My Group ID :%d\n",         my_info -> pw_gid);
            printf("My Real Name:%s\n",         my_info -> pw_gecos);
            printf("My Home Dir :%s\n",         my_info -> pw_dir);
            printf("My Work Shell:%s\n",        my_info -> pw_shell);
        }

        // 测试整形最大值
        long long test_16 = 0x1000000000000000;
        printf( "%lld\n", test_16 );
        printf( "%p\n", &test_16);
        return EXIT_SUCCESS;
    }/*}}}*/

    // 相当于直接把 src/struct.c 文件里所有的代码拷贝到这里，所谓的预处理嘛
    #include "src/struct.c"

    // 函数指针
    if( strcmp( "func_point", argv[1] ) == 0)
    {/*{{{*/
        typedef int (*fun_ptr)(int,int); // 定义一种函数指针类型
        fun_ptr f  = max; // 用该类型声明一个函数指针，并将该函数指针指向函数
        int b      = f(10,29);
        printf("%d\n",b);

        int (*f2)(int,int) = max; // 也可以直接声明一个函数指针，指向函数
        printf("%d\n",f2(4,65));

        // 测试下回掉函数
        printf("%d\n",call_func(max)); // 将max函数作为参数传进去
    }/*}}}*/

    // 测试缓冲区
    // 1.碰见 \n
    // 2.缓冲区满
    // 3.碰见输出
    if( strcmp("io-cache",argv[1]) == 0 )
    {/*{{{*/
        printf("test io-cache !");
        printf("test io-cache !");
        int input;
        scanf("%d",&input);
        sleep( 2 );
        return EXIT_SUCCESS;
    }/*}}}*/

    // node 链表操作
    if(strcmp("node",argv[1]) == 0)
    {/*{{{*/
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
        return EXIT_SUCCESS;
    }/*}}}*/

    // 队列
    if (strcmp("queue",argv[1]) == 0)
    {/*{{{*/
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
        return EXIT_SUCCESS;
    }/*}}}*/

    // 队列
    if (strcmp("queue-qq",argv[1]) == 0)
    {/*{{{*/
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
        return EXIT_SUCCESS;
    }/*}}}*/

    // 快速排序
    if(strcmp("quick_sort",argv[1]) == 0)
    {/*{{{*/
        // int test_data[] = {10,1,2,6,8,7,5,9,3,16,15,12,4,14,11,13};
        int test_data[] = {2,1,3,5,4};
        int lenth = sizeof(test_data) / sizeof(int);
        printf("快排演示 %d \n",lenth);

        for(int i = 0; i < sizeof(test_data) / sizeof(int); i++){
            printf("%d\t",test_data[i]);
        }
        printf("\n");
        sort_in_quick(test_data,0,lenth - 1);

        // 第二种实现
        int arr[] = {30,1,34,53,2,46,5,30,8,9,100,30,28,18,90,78,30,56,84,33,30,20};
        int arr_size = sizeof(arr) / sizeof(arr[0]);
        divide(arr,0,arr_size -1);
        return EXIT_SUCCESS;
    }/*}}}*/

    // 线程退出
    if(strcmp("thread_exit",argv[1]) == 0)
    {/*{{{*/
        pthread_t tid1 , tid2 , tid3;
        void **res;
        pthread_create(&tid1,NULL,first_thread,NULL);
        pthread_create(&tid2,NULL,second_thread,NULL);
        pthread_create(&tid3,NULL,third_thread,NULL);
        pthread_join( tid1 , res ); // 获取进程退出资源
        sleep(10);
    }/*}}}*/

    // thread access
    if(strcmp("thread_access",argv[1]) == 0)
    {/*{{{*/
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
        return EXIT_SUCCESS;
    }/*}}}*/

    // 线程与进程
    if(strcmp("thread",argv[1]) == 0)
    {/*{{{*/
        printf("当前进程ID : %u\n",getpid());
        printf("当前进程父ID : %u\n",getppid());
        printf("当前用户ID : %u\n",getuid());
        printf("当前有效用户ID : %u\n",geteuid());
        printf("当前组ID : %u\n",getgid());
        printf("当前有效组ID : %u\n",getegid());
        printf("-----------------\n\n");

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
        return EXIT_SUCCESS;
    }/*}}}*/

    // 从指定的消息队列中读出数据
    if( strcmp("ipc-queue-rcv",argv[1]) == 0)
    {/*{{{*/
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
    if(strcmp("ipc-queue",argv[1]) == 0)
    {/*{{{*/
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
    if(strcmp("brother-pipe",argv[1]) == 0)
    {/*{{{*/
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
    if(strcmp("pipe",argv[1]) == 0)
    {/*{{{*/
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

    // no-zombie
    if(strcmp("no-zombie",argv[1]) == 0)
    {/*{{{*/
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
    if(strcmp("zombie",argv[1]) == 0)
    {/*{{{*/
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
    if(strcmp("wait",argv[1]) == 0)
    {/*{{{*/
        pid_t pid = fork();
        int num,status;
        if(pid < 0){
            perror("fail to fork");
            exit(1);
        }else if(pid == 0){
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
    }/*}}}*/

    // 多进程操作
    if(strcmp("proc",argv[1]) == 0)
    {/*{{{*/
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
    if(strcmp("vfork",argv[1]) == 0)
    {/*{{{*/
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
    }/*}}}*/

    // 移位操作
    if(strcmp("shift",argv[1]) == 0)
    {/*{{{*/
        int a = 12;
        a = a >> 2;
        printf("a : %d \n",a);
    }/*}}}*/

    // 测试EOF
    if(strcmp("eof",argv[1]) == 0)
    {/*{{{*/
        char test_eof;
        while( (test_eof = getchar()) != EOF){
            putchar(test_eof);
        }
    }/*}}}*/

    // 测试fgets()
    if(strcmp("fgets",argv[1]) == 0)
    {/*{{{*/
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
    }/*}}}*/

    // 指向多维数组的指针
    if( strcmp("multi-array",argv[1]) == 0 )
    {/*{{{*/
        int zippo[3][2] = {
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
    }/*}}}*/

    // 测试下字符串数组
    if( strcmp( "str-array", argv[1] ) == 0 )
    {/*{{{*/
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

        char ar[] = MSG;
        char *pt = MSG;
        printf("MSG : %p \n",MSG); // 字面量存储的位置
        printf("ar : %p \n",ar); // 数组存储的位置
        printf("pt : %p \n",pt); // 指针指向的位置
        printf("-----------------\n\n");

        // 处理多维数组的函数
        int zippo[3][2] =
        {
            {2,3},
            {4,5},
            {6,7}
        };
        sum_rows(zippo,3);
    }/*}}}*/

    // 使用预定义宏
    if( strcmp( "default-macro", argv[1] ) == 0 )
    {/*{{{*/
        printf("File :%s\n", __FILE__ );
        printf("Date :%s\n", __DATE__ );
        printf("Time :%s\n", __TIME__ );
        printf("Line :%d\n", __LINE__ );
        printf("ANSI :%d\n", __STDC__ );
        printf("AUTHOR: %s \n---------------\n\n","codekissyoung");
    }/*}}}*/

    // 读取文件
    if(strcmp("read-file",argv[1]) == 0)
    {/*{{{*/
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
    }/*}}}*/

    // 不定参数
    if( strcmp("uncertain-var",argv[1]) == 0 )
    {/*{{{*/
        print_args(-1,"hello","world",NULL);
        print_args(-1,"Olympic","china","Beijing",NULL);
    }/*}}}*/

    // 进程
    if( strcmp("fork",argv[1]) == 0 )
    {/*{{{*/
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
    }/*}}}*/

    // exec 函数
    if( strcmp("exec",argv[1]) == 0 )
    {/*{{{*/
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
        system("ls -alh");
    }/*}}}*/

    // wait3 函数
    if( strcmp("wait3",argv[1]) == 0)
    {/*{{{*/
        pid_t pid;
        int status;
        struct rusage rusage;

        pid = fork();
        if(pid < 0)
        {
            exit(1);
        }
        else if(pid == 0)
        {
            printf("the child \n");
            exit(0);
        }
        else
        {
            printf("the parent \n");
            if(wait3(&status,0,&rusage) == -1)
            {
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
    }/*}}}*/
    // ------------------------------------------ end --------------------------------------------- //
    return EXIT_SUCCESS;
}

