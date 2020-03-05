#ifndef _BUFFER_H
#define _BUFFER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "event.h"
#include "protocol.h"

typedef struct io_buff io_buff;
struct io_buff {
    int sfd;            /* socket file description */
    //char *rbuff;      /* read buffer */
    //char *wbuff;      /* write buffer */
    req_rsp_pack *rbuff; /* read buffer */
    req_rsp_pack *wbuff; /* write buffer */
    int rsize; /* read buffer size */
    int wsize; /* write buffer size */
    int all_len;   /* need to read or write length(all) */
    int lft_len;   /* need to read or write length(left) */
    struct event_base *event_handle; /* event drive handle */
    io_buff *next;  /* next io_buff item */
};

typedef struct buff_queue buff_queue;
struct buff_queue {
    io_buff *head;
    io_buff *tail;
    pthread_mutex_t lock;
};

/*
 *  initialize freelist
 */
void freelist_init(void);
/*
 *  get buffer item from freelist
 */
io_buff *buff_from_freelist(void);
/*
 *  add buff item to freelist
 */
void buff_add_to_freelist(io_buff *buff);
/*
 *  increase buffer size
 */
int buff_increase(char *buff, int old_size, int new_size);
/*
 *  free the buffer
 */
void buff_free();

/*
 *  initialize buffer queue
 */
void buff_queue_init(buff_queue *bq);
/*
 *  pop buffer queue item 
 */
io_buff *buff_queue_pop(buff_queue *bq);
/*
 *  push buffer queue item 
 */
void buff_queue_push(buff_queue *bq, io_buff *buff);

#endif
