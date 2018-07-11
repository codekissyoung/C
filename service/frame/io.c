#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "event.h"
#include "buffer.h"
#include "thread.h"
#include "server.h"
#include "log.h"

/* alloc connection item count one time */
#define CONNS_PER_ALLOC     64

/* connection defination */
typedef struct conn_queue_item cq_item;
struct conn_queue_item {
    int sfd;                /* socket file description */
    int event_flag;         /* monitor event */
    enum io_cmd cmd;        /* command of io thread */
    struct io_buff *buff;   /* buffer poniter(for write command) */
    cq_item *next;          /* next item */
};

/* connection queue defination */
typedef struct conn_queue conn_queue;
struct conn_queue {
    cq_item *head;          /* queue head */
    cq_item *tail;          /* queue tail */
    pthread_mutex_t lock;
};

/* io thread defination */
struct io_thread {
    pthread_t tid;      /* thread id */
    conn_queue *cq;     /* connection queue */
    int pipe_read_fd;   /* file description of pipe read(for notify thread) */
    int pipe_write_fd;  /* file description of pipe write */
    struct event_base *event_handle;    /* event drive handle */
};

/* thread array */
struct io_thread *io_threads;

/* free connect list */
static cq_item *conn_freelist;
static pthread_mutex_t conn_freelist_lock;

/* thread init lock. be sure all thread initial ok */
static pthread_mutex_t init_lock;
static pthread_cond_t init_cond;
static int init_cnt = 0;

/* forward declaration */
void dispatch_io(int fd, int event_flag, enum io_cmd cmd, struct io_buff *buff);
void dispatch_read(int fd, struct io_buff *buff);

/*
 *  initialize connection queue
 *  each io thread have a connection queue.
 */
static void cq_init(conn_queue *cq) {
    pthread_mutex_init(&cq->lock, NULL);
    cq->head = NULL;
    cq->tail = NULL;
}

/*
 *  put a connection to connection queue.
 *      then the connection will be processed.
 */
static void cq_push(conn_queue *cq, cq_item *item) {
    item->next = NULL;
    pthread_mutex_lock(&cq->lock);
    if (cq->tail == NULL) {
        cq->head = item;
    }
    else { 
        cq->tail->next = item;
    }
    cq->tail = item;
    pthread_mutex_unlock(&cq->lock);
}

/*
 *  pop a connection from connection queue. 
 *      each io thread have a connection queue.
 */
static cq_item *cq_pop(conn_queue *cq) {
    cq_item *item = NULL;
    pthread_mutex_lock(&cq->lock);
    item = cq->head;
    if (item != NULL) {
        cq->head = item->next;
        if (cq->head == NULL)
            cq->tail = NULL;
    }
    pthread_mutex_unlock(&cq->lock);

    return item;
}

/*
 *  increase free connection list
 */
static cq_item *conn_freelist_increase(void) {
    int i = 0;
    cq_item *item = NULL;
    
    /* allocate memory for new list */
    item = (cq_item *)calloc(CONNS_PER_ALLOC, sizeof(cq_item));
    if (item == NULL) {
        fprintf(stderr, "calloc error in file:%s line:%d\n", __FILE__, __LINE__);
        log_txt_err("calloc error");

        return NULL;
    }

    /* build all the item to a list(new list), 
        expect the first, because it will be return */
    for (i = 2; i < CONNS_PER_ALLOC; i++) {
        item[i - 1].next = item + i;
    }

    /* add the new list to 'conn_freelist_lock' */
    pthread_mutex_lock(&conn_freelist_lock);
    item[CONNS_PER_ALLOC - 1].next = conn_freelist;
    conn_freelist   = item + 1;
    pthread_mutex_unlock(&conn_freelist_lock);

    return item;
}

/*
 *  pop a connection from free connection list
 */
static cq_item *conn_from_freelist(void) {
    cq_item *item;
    pthread_mutex_lock(&conn_freelist_lock);
    item = conn_freelist;
    if (item != NULL) {
        conn_freelist = item->next;
    }
    pthread_mutex_unlock(&conn_freelist_lock);

    if (item == NULL) {
        item = conn_freelist_increase();
    }
    
    return item;
}

/*
 *  add connection to free connection list, when it was done.
 */
static void conn_add_to_freelist(cq_item *item) {
    //memset(item, 0, sizeof(cq_item));

    pthread_mutex_lock(&conn_freelist_lock);
    item->next = conn_freelist;
    conn_freelist = item;
    pthread_mutex_unlock(&conn_freelist_lock);
}

/*
 *  read data from client.
 */
static int conn_read(int fd, void *arg) {
    struct io_buff *buf = (io_buff *)arg;
    char *buff_ptr = (char *)(buf->rbuff) + buf->all_len - buf->lft_len;
    int ret = 0, read_size = 1024;

    while(1) {
        ret = read(fd, buff_ptr, read_size);

        /* if error occurred  */
        if (ret < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            else if (errno == EINTR) {
                log_txt_notice("read was interrupt by ENTR");
                continue;
            }
            else {
                log_txt_err("read error, fd:%d info:%s", fd, strerror(errno));

                close(fd);
                buff_add_to_freelist(buf);
                return -1;
            }
        }

        /* if socket was closed */
        else if (ret == 0) {
            log_txt_info("read return 0, socket:%d", fd);
            
            close(fd);  
            buff_add_to_freelist(buf);
            return -1;
        }

        log_txt_info("read %d bytes from fd:%d", ret, fd);
        log_bin_info(buff_ptr, ret);

        /* read ok, then check if finished */
        if (buf->all_len == 0) {
            buf->all_len = *(unsigned int *)buff_ptr;
            buf->lft_len = buf->all_len - ret;

            log_txt_info("pack all_len:%d left_len:%d fd:%d", buf->all_len, buf->lft_len, fd);
        } else {
            buf->lft_len -= ret;
        }

        if (buf->lft_len <= 0) {
            log_txt_info("read finish %d", fd);

            event_remove(buf->event_handle, fd, 0); 
            dispatch_task(buf);
            return 0;
        }

        read_size = buf->lft_len;
        buff_ptr = (char *)(buf->rbuff) + buf->all_len - buf->lft_len;
    }

    return 0;
}

/*
 *  write data to client
 */
static int conn_write(int fd, void *arg) {
    struct io_buff *buf = (io_buff *)arg;
    char *buff_ptr = (char *)(buf->wbuff) + buf->all_len - buf->lft_len;
    int ret = 0;

    while (1) {
        ret = write(fd, buff_ptr, buf->lft_len);

        /* if error occurred  */
        if (ret < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            else {  /* real error */
                log_txt_err("write error, info:%s", strerror(errno));

                close(fd);
                buff_add_to_freelist(buf);
                return -1;
            }
        }

        log_txt_info("write %d bytes to fd:%d", ret, fd);
        log_bin_info(buff_ptr, ret);

        /* write ok, then check if finished */
        buf->lft_len -= ret;
        if (buf->lft_len <= 0) {
            log_txt_info("write[conn_write] finished fd:%d, pack-length:%d", buf->sfd, buf->all_len);

            buf->all_len = 0;
            buf->lft_len = 0;
            event_mod(buf->event_handle, fd, EPOLLIN | EPOLLET, conn_read, (void *)buf);
            
            return 0;
        }

        buff_ptr += ret;
    }

    return 1;
}

/*
 * write response
 */
int write_rsp(io_buff *buf) {
    int ret = write(buf->sfd, (char *)buf->wbuff, buf->lft_len);
    if (ret < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return 1;
        else {
            log_txt_err("write error, socket:[%d]", buf->sfd);
            close(buf->sfd);
            buff_add_to_freelist(buf);
            return -1;
        }
    }

    buf->lft_len -= ret;
    if (buf->lft_len <= 0) {
        log_txt_info("write[write_rsp] finished fd:%d, pack-length:%d", buf->sfd, buf->all_len);
    
        buf->all_len = 0;
        buf->lft_len = 0;
        dispatch_read(buf->sfd, buf);

        return 0;
    }

    return 1;
}

/*
 *  set event for a connection
 */
static int set_event(struct event_base *event_handle, cq_item *item) {
    struct io_buff *buff;
    int ret = 0;

    buff = item->buff;
    if (buff == NULL) {
        fprintf(stderr, "buffer is null in file:%s line:%d\n", __FILE__, __LINE__);
        log_txt_err("buffer is null");

        close(item->sfd);
        return 0;
    }
    switch (item->cmd) {
        case io_cmd_read:
            ret = event_add(event_handle, item->sfd, item->event_flag, conn_read, (void *)buff);
            break;

        case io_cmd_write:
            ret = event_add(event_handle, item->sfd, item->event_flag, conn_write, (void *)buff);
            break;
        
        case io_cmd_state:
            break;

        case io_cmd_stop:
            /* free event */
            event_free(event_handle);

            /* notify thread will stop */
            pthread_mutex_lock(&init_lock);
            init_cnt++;
            pthread_cond_signal(&init_cond);
            pthread_mutex_unlock(&init_lock);

            /* exit thread */
            pthread_exit(NULL);
            break;

        default:
            ret = -2;
    }

    return ret;
}

/*
 *  process notification
 */
static int process_conn(int fd, void *arg) {
    struct io_thread *me = (struct io_thread *)arg;
    cq_item *item;
    char buf[1];

    /* read data from pipe */
    if (read(fd, buf, 1) != 1) {
        log_txt_err("read from pipe failed, fd:%d", fd);
    }

    item = cq_pop(me->cq);
    assert(item);

    if (set_event(me->event_handle, item) < 0) {
        log_txt_err("set_event error");

        close(item->sfd);
    }

    conn_add_to_freelist(item);
    return 0;
}

/*
 *  thread entrance
 */
static void *worker_event(void *arg) {
    struct io_thread *me = (struct io_thread *)arg;

    pthread_mutex_lock(&init_lock);
    init_cnt++;
    pthread_cond_signal(&init_cond);
    pthread_mutex_unlock(&init_lock);

    event_loop(me->event_handle);
    return NULL;
}

/*
 *  record last dispath result.
 */
static unsigned int last_thread = 0;

/*
 *  dispatch a connection to a io thread.
 */
void dispatch_io(int fd, int event_flag, enum io_cmd cmd, struct io_buff *buff) {
    cq_item *conn = conn_from_freelist();
    int idx = last_thread++ % setting.io_thread_num;
    struct io_thread *thread = io_threads + idx;

    if (conn == NULL) {
        log_txt_err("conn_from_freelist return NULL");

        close(fd);
    }

    buff->event_handle = thread->event_handle;
    buff->sfd = fd;
    conn->sfd = fd;
    conn->event_flag = event_flag;
    conn->cmd = cmd;
    conn->buff = buff;
    cq_push(thread->cq, conn);

    if (write(thread->pipe_write_fd, "", 1) != 1) {
        log_txt_err("write to pipe failed, fd %d", thread->pipe_write_fd);
    }

    log_txt_info("new io cmd, fd:%d cmd:%d", fd, cmd);
}

void dispatch_read(int fd, struct io_buff *buff) {
    int idx = last_thread++ % setting.io_thread_num;
    struct io_thread *thread = io_threads + idx;
    int ret = 0;

    buff->event_handle = thread->event_handle;
    buff->sfd = fd;

    ret = event_add(buff->event_handle, buff->sfd, EPOLLIN | EPOLLET, conn_read, (void *)buff);
    if (ret < 0) {
        log_txt_err("add read event failed, fd:%d", buff->sfd);
    }

    log_txt_info("add read event, fd:%d", fd);
}

void dispatch_write(int fd, struct io_buff *buff) {
    int idx = 0;
    struct io_thread *thread = NULL;
    int ret = 0;

    buff->all_len = buff->wbuff->len;
    buff->lft_len = buff->wbuff->len;
    buff->sfd = fd;

    if (write_rsp(buff) == 0) {
        return ;
    }

    idx = last_thread++ % setting.io_thread_num;
    thread = io_threads + idx;
    buff->event_handle = thread->event_handle;

    ret = event_add(buff->event_handle, fd, EPOLLOUT | EPOLLET, conn_write, (void *)buff);
    if (ret < 0) {
        log_txt_err("add write event failed, fd:%d", buff->sfd);
    }

    log_txt_info("add write event, fd:%d", fd);
}

/* 
 *  setup thread
 */
static void setup_thread(struct io_thread *thread) {
    int fd[2];
    pthread_t tid;
    pthread_attr_t attr;

    /* initial connection queue */
    thread->cq = (conn_queue *)malloc(sizeof(struct conn_queue));
    if (thread->cq == NULL) {
        log_txt_err("malloc failed, size:%zu", sizeof(struct conn_queue));
        exit(1);
    }
    cq_init(thread->cq);
    
    /* create pipe */
    if (pipe(fd)) {
        log_txt_err("create pipe failed");

        exit(1);
    }
    thread->pipe_read_fd    = fd[0];
    thread->pipe_write_fd   = fd[1];

    /* set event */
    thread->event_handle    = event_init();
    if (event_add(thread->event_handle, thread->pipe_read_fd,
                    EPOLLIN /*| EPOLLET*/, process_conn, thread) < 0) {
        log_txt_err("add pipe fd to event failed");

        exit(1);
    }

    /* create thread */
    pthread_attr_init(&attr);
    if (pthread_create(&tid, &attr, worker_event, (void *)thread) < 0) {
        log_txt_err("pthread_create failed");

        exit(1);
    }
}

void io_thread_init(int nthreads) {
    int i = 0;

    pthread_mutex_init(&init_lock, NULL);
    pthread_cond_init(&init_cond, NULL);

    pthread_mutex_init(&conn_freelist_lock, NULL);
    conn_freelist = NULL;

    /* allocate memory, to store thread array  */
    io_threads = (struct io_thread *)malloc(nthreads * sizeof(struct io_thread));
    if (io_threads == NULL) {
        log_txt_err("malloc error");
        exit(1);
    }
    
    /* create thread */
    for (i = 0; i < nthreads; i++) {
        setup_thread(&io_threads[i]);
    }

    /* wait for all thread create ok */
    pthread_mutex_lock(&init_lock);
    while (init_cnt < nthreads) {
        pthread_cond_wait(&init_cond, &init_lock);
    }
    pthread_mutex_unlock(&init_lock);

    log_txt_notice("io thread setup ok");
}

void io_thread_stop() {
    int i = 0;  
    struct io_buff tmp;
    init_cnt = 0;

    /* send stop command */
    for (i = 0; i < setting.io_thread_num; i++) {
        dispatch_io(-1, 0, io_cmd_stop, &tmp);
    }

    /* wait for all the threads stop */
    pthread_mutex_lock(&init_lock);
    while(init_cnt < setting.io_thread_num) {
        pthread_cond_wait(&init_cond, &init_lock);
    }
    pthread_mutex_unlock(&init_lock);

    log_txt_notice("io thread stop ok");
}

