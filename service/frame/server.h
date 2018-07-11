#ifndef _SERVER_H
#define _SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dlfcn.h>

#define PACKAGE "open server"
#define VERSION "1.0"

#define setting_path_len 255
#define setting_name_len 64

enum io_cmd {
    io_cmd_read,
    io_cmd_write,
    io_cmd_state,
    io_cmd_stop
};

/* module infomation */
struct mod_info {
    char mod_name[setting_name_len];
    char mod_file[setting_path_len];
    char mod_conf[setting_name_len];
    unsigned int cmd_begin;
    unsigned int cmd_end;
    unsigned int magic_code;
    char init_func_name[setting_name_len];
    char proc_func_name[setting_name_len];
    char uninit_func_name[setting_name_len];

    void *mod_id;
    int (*init_func)(char *);
    //int (*proc_func)(const char *, char *, int);
    //int (*proc_func)(const req_rsp_pack*, req_rsp_pack*, int);
    int (*proc_func)(struct io_buff*);
    int (*uninit_func)();
};

/* server settings */
struct setting {
    char ip[50];
    int port;
    int buff_size;
    int max_client;
    int io_thread_num;
    int proc_thread_num;
    int proc_thread_async; /* 工作线程是为异步 */
    int backlog;

    /* log setting */
    char log_path[255];
    int log_level;
    int log_file_size;

    int mod_num;
    struct mod_info *mod;
};

/* forward declaration about settings */
extern struct setting setting;

/*
 *  get module by command number
 */
struct mod_info *get_mod(unsigned int cmd);


#endif

