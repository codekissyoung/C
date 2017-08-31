if(strcmp("struct",argv[1]) == 0){
	// 声明结构体变量
	struct Books b1;
	// b1.title = "How do I love you"; // 这种方式是错误的，数组名直接接收字符串？？？
	strcpy(b1.title,"How Do I Love you\n");
	strcpy(b1.author,"codekissyoung\n");
	strcpy(b1.subject,"i love you\n");
	b1.id = 23334235;
	printBook(&b1); // 取地址符 取出该结构体首地址，传给函数里面使用，从而处理该地址处的数据
}

