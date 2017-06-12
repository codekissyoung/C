#include "../include/common.h"

pthread_t tid;

// 第一个线程函数
void *first_thread(void *arg){
	printf("first thread ! \n");
	return (void *)1;
}

void *second_thread(void *arg){
	printf("second thread ! \n");
	pthread_exit((void *)3);
	return (void *)1;
}

void *third_thread(void *arg){
	printf("third thread ! sleep 5s \n");
	sleep(5);
	return NULL;
}


