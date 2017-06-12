/* 栈 */
struct stack{
	int data[1000];
	int top;
};

/* 链表 */
struct node{
	int data;
	struct node *next;
};

// 链表
struct test_struct{
	int array[2];
	char ch;
};

typedef struct test_struct Test;

/* 队列 */
struct queue {
	int data[1000];
	int head;
	int tail;
};
struct arg_struct{
	char arg1[10];
	int arg2;
	float arg3;
};

typedef struct arg_struct ARG;

typedef struct heap_stack{
	int *heap;
	int *stack;
} HS;

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

