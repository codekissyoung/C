#ifndef _THREAD_H
#define _THREAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "protocol.h" 
#include "buffer.h" 

typedef struct proc_thread proc_thread;
struct proc_thread {
	pthread_t tid;			/* thread id */
	buff_queue	bq;			/* buffer queue */
	int bq_len;				/* buffer queue length */
	pthread_cond_t cond;	/* condition variable, to hung up */
	pthread_mutex_t	lock;	/* lock */
	int stop;				/* if thread will be stop */
    char is_async;          /* thread whill be asynchronous */
	proc_thread *next;		/* next thread */
};

void thread_init(int nthreads, int is_async);

void dispatch_task(io_buff *buff);

void thread_stop();

#endif
