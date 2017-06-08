// 全局变量定义
#ifndef GLOBAL
	#define GLOBAL 1
	int b1 = 14;
	float PI = 3.14;
	Test var = {{0x12345678,0x98765432},0x30};

	// 结构化数据
	struct book{
		char title[40];
		char author[40];
		float value;
	};

	struct msg{
		long msg_types;
		char msg_buf[511];
	};

	#define BUFSZ 4096
#endif




float a1 = 23.29; // a1 变量定义
int global = 2;




