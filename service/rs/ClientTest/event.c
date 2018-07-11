#include "event.h"
#include "log.h"
#include <pthread.h>

struct event_base *event_init() {
    int file_num = EVENT_NUM;
    struct event_base *base;
    struct rlimit rl;

    /* 为base分配内存 */
    base = (struct event_base *)malloc(sizeof(struct event_base));
    if (base == NULL) {
        fprintf(stderr, "malloc error\n");
        log_txt_err("malloc error");

        return NULL;
    }
    memset(base, 0, sizeof(struct event_base));
    
    /* 获取打开最大文件数 */
    if (getrlimit(RLIMIT_NOFILE, &rl) == 0 &&
        rl.rlim_cur != RLIM_INFINITY) {
        file_num = rl.rlim_cur - 1;
    }

    /* 初始化epoll */
    base->event_cnt = file_num;
    base->epoll_fd = epoll_create(file_num);
    if (base->epoll_fd == -1) {
        fprintf(stderr, "epoll_create error\n");
        log_txt_err("epoll_create error");

        free(base);
        return NULL;
    }

    /* 初始化事件队列 */
    base->events = (struct event *)malloc(file_num * sizeof(struct event));
    if (base->events == NULL) {
        fprintf(stderr, "malloc error\n");
        log_txt_err("malloc error");

        free(base);
        return NULL;
    }

    return base;
}

int event_add(struct event_base *base, int fd, int event, int (*func)(int, void *), void *arg) {
    struct epoll_event epoll_ev;

    /* 添加到事件队列 */
    assert(base);
    assert(base->events);
    base->events[fd].fd     = fd;
    base->events[fd].event  = event;
    base->events[fd].func   = func;
    base->events[fd].arg    = arg;
    base->events[fd].next   = NULL;

    /* 添加事件到epoll */
    epoll_ev.data.ptr = base->events + fd;
    epoll_ev.events  = event;
    if (epoll_ctl(base->epoll_fd, EPOLL_CTL_ADD, fd, &epoll_ev) < 0) {
        fprintf(stderr, "epoll_ctl[EPOLL_CTL_ADD] error, fd:%d msg:%s\n", fd, strerror(errno));
        log_txt_err("epoll_ctl[EPOLL_CTL_ADD] error");

        return -1;
    }
    return 0;
}

int event_mod(struct event_base *base, int fd, int event, int (*func)(int, void *), void *arg) {
    struct epoll_event epoll_ev;

    base->events[fd].event  = event;
    base->events[fd].func   = func;
    base->events[fd].arg    = arg;

    /* epoll事件 */
    epoll_ev.data.ptr = base->events + fd;
    epoll_ev.events  = event;

    if (epoll_ctl(base->epoll_fd, EPOLL_CTL_MOD, fd, &epoll_ev) < 0) {
        fprintf(stderr, "epoll_ctl[EPOLL_CTL_MOD] error\n");
        log_txt_err("epoll_ctl[EPOLL_CTL_MOD] error, info:%s", strerror(errno));

        return -1;
    }
    return 0;
}

int event_remove(struct event_base *base, int fd, int event) {
    struct epoll_event epoll_ev;

    /* 移除epoll事件 */
    epoll_ev.data.ptr = NULL;
    epoll_ev.events  = 0;

    if (epoll_ctl(base->epoll_fd, EPOLL_CTL_DEL, fd, &epoll_ev) < 0) {
        fprintf(stderr, "epoll_ctl[EPOLL_CTL_DEL] error\n");
        log_txt_err("epoll_ctl[EPOLL_CTL_DEL] error, info:%s", strerror(errno));

        return -1;
    }

    return 0;
}

int event_loop(struct event_base *base) {
    int ev_cnt = 0;
    int i = 0; //active_fd = -1;
    struct event *ev;

    assert(base);
    assert(base->epoll_fd);

    struct epoll_event activ_events[100];

    while (1) {
        ev_cnt = epoll_wait(base->epoll_fd, activ_events, 100, 1000);
        if (ev_cnt == -1) {
            continue;
        }
        for (i = 0; i < ev_cnt; i++) {
            ev = (struct event*)activ_events[i].data.ptr;
            ev->func(ev->fd, ev->arg);
        }
    }

    return 0;
}

int event_free(struct event_base *base) {

    if (base->events)
        free(base->events);
    if (base)
        free(base);

    return 0;
}

