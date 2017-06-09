#include "../include/thread.h"

// 第一个线程函数
void *first_thread(void *arg){
	printf("first thread ! \n");
	return (void *)1;
}

void *second_thread(void *arg){
	printf("second thread ! \n");
	return (void *)1;
}

void *third_thread(void *arg){
	printf("third thread ! \n");
	return (void *)1;
}
