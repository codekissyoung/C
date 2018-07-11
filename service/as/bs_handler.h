#ifndef __BS_HANDLER_H_
#define __BS_HANDLER_H_

#include "buffer.h"
#include "common.h"
#include "interface.h"

// bs接口, 该接口多线程安全

typedef struct {
    char _host[MAX_SVR_HOST_LEN];
    short _port;
} bs_single_conf_t;

typedef struct {
    int _bak_num; // bs副本实际数量
    bs_single_conf_t _conf[MAX_NODE_NUM_PER_GROUP];
} bs_group_conf_t;

typedef struct {
    bs_group_conf_t _conf;
    int _socket[MAX_NODE_NUM_PER_GROUP];
    char _health_flag[MAX_NODE_NUM_PER_GROUP];
    int _cur_idx;

    //as2bs_get_post_t *_send_buff;
    //bs2as_get_post_t *_recv_buff;
    req_pack_t *_send_buff;
    rsp_pack_t *_recv_buff;
    size_t _send_buff_size;
    size_t _recv_buff_size;
    int _lft_len; // left length of read or write
    int _read_len; // only read when reading data from network
    char _failed; // if error eccured
} bs_svr_t;

typedef struct {
    int _bs_group_num;
    int _bs_node_num;
    bs_svr_t *_bs_svr;
    int _epoll_fd;
    struct epoll_event *_epoll_events;
} bs_service_t;

typedef struct {
    int _user_num;

    int *_user_types;

    uint64_t *_user_ids;
    char *_tag;
    int _start_idx;
    int _req_num;
} bs_get_post_req_t;

typedef struct {
    int _list_num;

    int _user_types[MAX_FOLLOWER_NUM][MAX_RET_POST_NUM];
    uint64_t _user_ids[MAX_FOLLOWER_NUM][MAX_RET_POST_NUM];

    uint64_t _lists[MAX_FOLLOWER_NUM][MAX_RET_POST_NUM];
    int _list_len[MAX_FOLLOWER_NUM];
} bs_get_post_rsp_t;

typedef struct {
    int _user_num;
    uint64_t *_user_ids;
    char *_tag;
} bs_get_post_cnt_req_t;

typedef struct {
    uint64_t _post_id;
    int _user_num;
    uint64_t _user_ids[MAX_REF_CNT_PER_POST+1];
    int _tag_num;
    char *_tags[MAX_TAG_CNT_PER_POST];
} bs_set_post_req_t;

// 初始化bs，建立bs连接
bs_service_t *bs_init(bs_group_conf_t *bs_conf, int bs_group_num);

// 获取post list，需要外部构造查询条件bs_get_post_req_t
// 返回结果保存到rsp中，在外部分配内存
int bs_get_post(bs_service_t *bsc, bs_get_post_req_t *req, bs_get_post_rsp_t *rsp, int max_time);

//added by Radio
int bs_get_post_by_page(bs_service_t *bsc, bs_get_post_req_t *req, bs_get_post_rsp_t *rsp, int max_time);

// 获取post长度
int bs_get_post_cnt(bs_service_t *bsc, bs_get_post_cnt_req_t *req, int max_time);

// 发表文章
int bs_set_post(bs_service_t *bsc, bs_set_post_req_t *req, int set_post_user_type,int max_time);

// 断开连接
void bs_free(bs_service_t *bsc);

#endif

