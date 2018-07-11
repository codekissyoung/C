#include "buffer.h"
#include "server.h"
#include "log.h"

/* alloc buffer item count one time */
#define BUFFS_PER_ALLOC 100

/* free buffer list */
static io_buff *buff_freelist   = NULL;
static pthread_mutex_t freelist_lock;

/*
 *  initialize freelist
 */
void freelist_init(void) {
    int item_cnt = 200, i = 0;
    buff_freelist = (io_buff *)calloc(item_cnt, sizeof(io_buff));
    if (buff_freelist == NULL) {
        fprintf(stderr, "calloc error in file:%s line:%d\n", __FILE__, __LINE__);
        log_txt_err("calloc error");

        exit(1);
    }

    /* allocate memory for read and write */
    for (i = 0; i < item_cnt; i++) {
        buff_freelist[i].rbuff = (req_rsp_pack *)malloc(setting.buff_size);     
        if (buff_freelist[i].rbuff == NULL) {
            fprintf(stderr, "malloc error in file:%s line:%d\n", __FILE__, __LINE__);
            log_txt_err("malloc error");

            exit(1);
        }
        memset(buff_freelist[i].rbuff, 0, setting.buff_size);
        buff_freelist[i].rsize = setting.buff_size;

        buff_freelist[i].wbuff = (req_rsp_pack *)malloc(setting.buff_size);     
        if (buff_freelist[i].wbuff == NULL) {
            fprintf(stderr, "malloc error in file:%s line:%d\n", __FILE__, __LINE__);
            log_txt_err("malloc error");

            exit(1);
        }
        memset(buff_freelist[i].wbuff, 0, setting.buff_size);
        buff_freelist[i].wsize = setting.buff_size;
    }

    /* build all the item to a list */
    for (i = 1; i < item_cnt; i++) {
        buff_freelist[i - 1].next = buff_freelist + i;
    }

    /* initialize lock */
    pthread_mutex_init(&freelist_lock, NULL);
}

/*
 *  increase free buffer list
 */
static io_buff *buff_freelist_increase() {
    int i = 0;
    io_buff *new_list= NULL;
    
    /* allocate memory for new list */
    new_list = (io_buff *)calloc(BUFFS_PER_ALLOC, sizeof(io_buff));
    if (new_list == NULL) {
        fprintf(stderr, "calloc error in file:%s line:%d\n", __FILE__, __LINE__);
        log_txt_err("calloc error");

        return NULL;
    }

    /* allocate memory for read and write */
    for (i = 0; i < BUFFS_PER_ALLOC; i++) {
        new_list[i].rbuff = (req_rsp_pack *)malloc(setting.buff_size);  
        if (new_list[i].rbuff == NULL) {
            fprintf(stderr, "malloc error in file:%s line:%d\n", __FILE__, __LINE__);
            log_txt_err("malloc error");

            exit(1);
        }
        memset(new_list[i].rbuff, 0, setting.buff_size);
        new_list[i].rsize = setting.buff_size;

        new_list[i].wbuff = (req_rsp_pack *)malloc(setting.buff_size);  
        if (new_list[i].wbuff == NULL) {
            fprintf(stderr, "malloc error in file:%s line:%d\n", __FILE__, __LINE__);
            log_txt_err("malloc error");

            exit(1);
        }
        memset(new_list[i].wbuff, 0, setting.buff_size);
        new_list[i].wsize = setting.buff_size;
    }

    /* build all the item to a list(new list), 
        expect the first, because it will be return */
    for (i = 2; i < BUFFS_PER_ALLOC; i++) {
        new_list[i - 1].next = new_list + i;
    }

    /* add the new list to 'conn_freelist_lock' */
    pthread_mutex_lock(&freelist_lock);
    new_list[BUFFS_PER_ALLOC - 1].next = buff_freelist;
    buff_freelist   = new_list + 1;
    pthread_mutex_unlock(&freelist_lock);

    return new_list;
}

/*
 *  get buffer item from freelist
 */
io_buff *buff_from_freelist(void) {
    io_buff *buff = NULL;
    pthread_mutex_lock(&freelist_lock);
    buff = buff_freelist;
    if (buff != NULL) {
        buff_freelist = buff->next;
    }
    pthread_mutex_unlock(&freelist_lock);

    if (buff == NULL) {
        buff = buff_freelist_increase();

        log_txt_info("buff_freelist_increase");
    }

    return buff;
}

/*
 *  add buff item to freelist
 */
void buff_add_to_freelist(io_buff *buff) {
    //memset(buff, 0, sizeof(io_buff));
    buff->sfd = 0;
    //memset(buff->rbuff, 0, buff->rsize);
    //memset(buff->wbuff, 0, buff->wsize);
    buff->all_len = 0;
    buff->lft_len = 0;
    buff->event_handle = NULL;
    buff->next = NULL;

    pthread_mutex_lock(&freelist_lock);
    buff->next = buff_freelist;
    buff_freelist = buff;
    pthread_mutex_unlock(&freelist_lock);
}

/*
 *  increase buffer size
 */
int buff_increase(char *buff, int old_size, int new_size) {
    char *old = buff;
    buff = (char *)malloc(new_size);
    if (buff) {
        memcpy(buff, old, old_size);
        free(old);
        return 0;
    }
    
    buff = old;
    return -1;  
}

/*
 *  free the buffer 
 */
void buff_free() {
    io_buff *ptr = buff_freelist;
    while(ptr) {
        free(ptr->rbuff);
        free(ptr->wbuff);
        ptr = ptr->next;
    }

    free(buff_freelist);
}

/*
 *  initialize buffer queue
 */
void buff_queue_init(buff_queue *bq) {
    bq->head = NULL;
    bq->tail = NULL;
    pthread_mutex_init(&bq->lock, NULL);
}

/*
 *  pop buffer queue item 
 */
io_buff *buff_queue_pop(buff_queue *bq) {
    io_buff *buff = NULL;
    pthread_mutex_lock(&bq->lock);
    buff = bq->head;
    if (buff != NULL) {
        bq->head = buff->next;
        if (bq->head == NULL)
            bq->tail = NULL;
    }
    pthread_mutex_unlock(&bq->lock);

    return buff;    
}

/*
 *  push buffer queue item 
 */
void buff_queue_push(buff_queue *bq, io_buff *buff) {
    buff->next = NULL;
    pthread_mutex_lock(&bq->lock);
    if (bq->tail == NULL) {
        bq->head = buff;
    }
    else 
        bq->tail->next = buff;
    bq->tail = buff;
    pthread_mutex_unlock(&bq->lock);
}
