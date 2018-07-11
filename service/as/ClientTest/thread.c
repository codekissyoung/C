#include "thread.h"
#include "server.h"
#include "log.h"

/* thread array */
static proc_thread *threads = NULL;

/* free thread list */
static proc_thread *thread_freelist = NULL;
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
        create_error_pack(buff->wbuff, ERR_MAGIC_CODE);
    }
    else {
        mod->proc_func(buff);
    }

    dispatch_write(buff->sfd, buff);
}

/*
 *  business process asynchronous
 */
static void thread_async_process(io_buff *buff) {
    unsigned magic = buff->rbuff->magic;
    int cmd = buff->rbuff->cmd;
    struct mod_info *mod = get_mod(cmd);

    /* error check */
    if (mod == NULL || mod->proc_func == NULL || magic != mod->magic_code) {
        create_error_pack(buff->wbuff, ERR_MAGIC_CODE);
        dispatch_write(buff->sfd, buff);

        return;
    }
            
    /* 处理逻辑 */
    mod->proc_func(buff);
}

/*
 *  thread entry
 */
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
            //log_txt_info("debug: bq_len:%d", me->bq_len);
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

/*
 *  create thread
 */
static void create_thread(proc_thread *thread) {
    pthread_t tid;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    if (pthread_create(&tid, &attr, thread_entry, (void *)thread) < 0) {
        fprintf(stderr, "pthread_create error in file:%s line:%d\n", __FILE__, __LINE__);
        log_txt_err("pthread_create error");

        exit(1);
    }

    memcpy(&thread->tid, &tid, sizeof(tid));
}

/*
 *  intialize thread pool
 */
void thread_init(int nthreads, int is_async) {
    int i = 0;
    pthread_mutex_init(&init_lock, NULL);
    pthread_cond_init(&init_cond, NULL);
    init_cnt = 0;
    thread_cnt = nthreads;

    /* allocate memory for thread pool */
    threads = (proc_thread *)calloc(nthreads, sizeof(proc_thread));
    if (threads == NULL) {
        fprintf(stderr, "calloc error in file:%s line:%d\n", __FILE__, __LINE__);
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
    //thread_freelist = threads;
    pthread_mutex_init(&freelist_lock, NULL);
    for (i = 2; i < nthreads; i++) {
        threads[i - 1].next = threads + i;
    }

    log_txt_err("business thread setup ok");
}

/*
 *  stop threads
 */
void thread_stop() {
    int i = 0;
    //proc_thread *thread;
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

#if 0
/*
 *  get thread from free thread list
 */
static proc_thread *thread_from_freelist() {
    proc_thread *thread;
    pthread_mutex_lock(&freelist_lock);
    thread = thread_freelist;
    if (thread != NULL) {
        thread_freelist = thread->next;
    }
    pthread_mutex_unlock(&freelist_lock);

    return thread;
}
#endif

/*
 *  add to free thread list.
 */
static void thread_add_to_freelist(proc_thread *thread) {
    pthread_mutex_lock(&freelist_lock);
    thread->next = thread_freelist;
    thread_freelist = thread;
    pthread_mutex_unlock(&freelist_lock);
}

/* last dispatch thread */
static unsigned int last_thread = 0;

/*
 *  dispatch task
 */
void dispatch_task(io_buff *buff) {
		proc_thread *thread;

#if 0   
    /* find free thread */
    thread = thread_from_freelist();

    /* have no free thread */
    if (thread == NULL) {
        int bq_min = threads->bq_len;
        int i = 0, min_idx = 0;
        for (; i < thread_cnt; i++) {
            if (bq_min > (threads + i)->bq_len) {
                bq_min = (threads + i)->bq_len;
                min_idx = i;
            }
            if (bq_min == 0) {
                thread = threads + i;
                break;
            }
        }
        thread = threads + min_idx;
    }
#endif  
    int idx = last_thread++ % thread_cnt;
    thread = threads + idx;
    
    /* push to buffer queue */
    buff_queue_push(&thread->bq, buff);
    pthread_cond_signal(&thread->cond);
}

