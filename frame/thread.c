#include "thread.h"
#include "server.h"
#include "log.h"

static proc_thread *threads = NULL;         /* thread array */
static proc_thread *thread_freelist = NULL; /* free thread list */
static pthread_mutex_t freelist_lock;

/* intialization lock */
static pthread_mutex_t init_lock;
static pthread_cond_t init_cond;
static int init_cnt = 0;

/* record thread count */
static int thread_cnt = 0;

/* forward declaration */
static void thread_add_to_freelist(proc_thread *thread);
void dispatch_write(int fd, struct io_buff *buff);

/*
 * create error pack header
 */
static void create_error_pack(req_rsp_pack *buff, int error) {
    buff->len = sizeof(pack_header);
    buff->magic = 0;
    buff->cmd = 0;
    buff->sequence= 0;
    buff->state = error;
}

/*
 *  business process
 */
static void thread_process(io_buff *buff) {
    unsigned magic = buff->rbuff->magic;
    int cmd = buff->rbuff->cmd;
    struct mod_info *mod = get_mod(cmd);

    /* error check */
    if (mod == NULL || mod->proc_func == NULL || magic != mod->magic_code) {
        if (mod == NULL) {
            log_txt_err("business module was null!");
        }
        else if (mod->proc_func == NULL) {
            log_txt_err("business module's proc function was null!");
        }
        else if (magic != mod->magic_code) {
            log_txt_err("magic_code not match, magic in request is:%d expect:%d", magic, mod->magic_code);
        }

        create_error_pack(buff->wbuff, ERR_MAGIC_CODE);
    }
    else {
        mod->proc_func(buff);
    }

    dispatch_write(buff->sfd, buff);
}

/*
 *  business process asynchronous
 *  这种模式下，框架不负责发送返回数据，需要有模块自己调用框架接口发送返回包
 */
static void thread_async_process(io_buff *buff) {
    unsigned magic = buff->rbuff->magic;
    int cmd = buff->rbuff->cmd;
    struct mod_info *mod = get_mod(cmd);

    /* error check */
    if (mod == NULL || mod->proc_func == NULL || magic != mod->magic_code) {
        if (mod == NULL) {
            log_txt_err("business module was null!");
        }
        else if (mod->proc_func == NULL) {
            log_txt_err("business module's proc function was null!");
        }
        else if (magic != mod->magic_code) {
            log_txt_err("magic_code not match, magic in request is:%d expect:%d", magic, mod->magic_code);
        }

        create_error_pack(buff->wbuff, ERR_MAGIC_CODE);
        dispatch_write(buff->sfd, buff);
        return;
    }
    /* 处理逻辑 */
    mod->proc_func(buff);
}

static void *thread_entry(void *arg) {
    proc_thread *me = (proc_thread *)arg;
    io_buff *buff = NULL;

    /* tell main thread intialize ok */
    if (!me->stop) {
        pthread_mutex_lock(&init_lock);
        init_cnt++;
        pthread_cond_signal(&init_cond);
        pthread_mutex_unlock(&init_lock);
    }

    while (!me->stop) {
        buff = buff_queue_pop(&me->bq);
        if (buff == NULL) {
            /* add itself to free thread list */
            thread_add_to_freelist(me);

            pthread_mutex_lock(&me->lock);
            pthread_cond_wait(&me->cond, &me->lock);
            pthread_mutex_unlock(&me->lock);
        } else {
            if (me->is_async) {
                thread_async_process(buff);
            }
            else {
                thread_process(buff);
            }
            me->bq_len--;
        }
    }

    buff = buff_queue_pop(&me->bq);
    while (buff) {
        free(buff->rbuff);  
        free(buff->wbuff);  
        free(buff);
        buff = buff_queue_pop(&me->bq);
    }
    
    /* thread will stop */
    pthread_mutex_lock(&init_lock);
    init_cnt++;
    pthread_cond_signal(&init_cond);
    pthread_mutex_unlock(&init_lock);

    return NULL;
}

static void create_thread(proc_thread *thread)
{
    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if (pthread_create(&tid, &attr, thread_entry, (void *)thread) < 0)
    {
        log_txt_err("pthread create error");
        exit(1);
    }
    memcpy(&thread->tid, &tid, sizeof(tid));
}

// 初始化线程池
void thread_init(int nthreads, int is_async)
{
    int i = 0;
    pthread_mutex_init(&init_lock, NULL);
    pthread_cond_init(&init_cond, NULL);
    init_cnt = 0;
    thread_cnt = nthreads;

    /* allocate memory for thread pool */
    threads = (proc_thread *)calloc(nthreads, sizeof(proc_thread));
    if (threads == NULL) {
        log_txt_err("calloc error");
        exit(1);
    }

    /* intialize threads */
    for (i = 0; i < nthreads; i++) {
        buff_queue_init(&threads[i].bq);
        threads[i].bq_len = 0;
        threads[i].is_async = (char)is_async;
        pthread_cond_init(&threads[i].cond, NULL);
        pthread_mutex_init(&threads[i].lock, NULL);
        threads[i].stop = 0;
    }
    
    /* create threads */
    for (i = 0; i < nthreads; i++) {
        create_thread(threads + i);
    }
    
    /* wait for all thread create ok */
    pthread_mutex_lock(&init_lock);
    while (init_cnt < nthreads) {
        pthread_cond_wait(&init_cond, &init_lock);
    }
    pthread_mutex_unlock(&init_lock);
    
    /* intialize freelist */
    pthread_mutex_init(&freelist_lock, NULL);
    for (i = 2; i < nthreads; i++) {
        threads[i - 1].next = threads + i;
    }

    log_txt_err("business thread setup ok");
}

void thread_stop()
{
    int i = 0;
    init_cnt = 0;

    for (i = 0; i < thread_cnt; i++) {
        threads[i].stop = 1;    
        pthread_cond_signal(&threads[i].cond);
    }

    /* wait for all thread stop */
    pthread_mutex_lock(&init_lock);
    while (init_cnt < thread_cnt) {
        pthread_cond_wait(&init_cond, &init_lock);
    }
    pthread_mutex_unlock(&init_lock);

    log_txt_err("business thread stop ok");
}

static void thread_add_to_freelist(proc_thread *thread) {
    pthread_mutex_lock(&freelist_lock);
    thread->next = thread_freelist;
    thread_freelist = thread;
    pthread_mutex_unlock(&freelist_lock);
}

/* last dispatch thread */
static unsigned int last_thread = 0;

void dispatch_task(io_buff *buff) {
    proc_thread *thread;
    int idx = last_thread++ % thread_cnt;
    thread = threads + idx;
    buff_queue_push(&thread->bq, buff);
    pthread_cond_signal(&thread->cond);
}

