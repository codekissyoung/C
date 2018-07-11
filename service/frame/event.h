#ifndef _EVENT_H
#define _EVENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/resource.h>
#include <assert.h>
#include <errno.h>

#define EVENT_NUM   3200

struct event {
    int fd;                     /* 注册的文件描述符 */
    int event;                  /* 发生的事件 */
    int (*func)(int, void *);   /* 发生事件时回调函数 */
    void *arg;                  /* 回调函数参数*/
    struct event *next;         /* 下一事件结构 */
};

struct event_base {
    int epoll_fd;                       /* epoll描述符 */
    int event_cnt;                      /* 事件数量 */
    struct event *events;               /* 监听事件队列 */
};

struct event_base *event_init();
int event_add(struct event_base *base, int fd, int event, int (*func)(int, void *), void *arg);
int event_mod(struct event_base *base, int fd, int event, int (*func)(int, void *), void *arg);
int event_loop(struct event_base *base);
int event_free(struct event_base *base);
int event_remove(struct event_base *base, int fd, int event);

#endif
