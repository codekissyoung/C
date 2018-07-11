#ifndef __REDIS_HANDLER_H_
#define __REDIS_HANDLER_H_

// 封装从redis查询post的接口
// redis_get_post_req_t中user_ids指向的内存由外部调用者管理，
// 因为大部分情况下这块内存可以直接复用协议报文中的内存
#include <stdint.h>
#include "hiredis.h"
#include "common.h"

typedef struct {
    char _host[MAX_SVR_HOST_LEN];
    short _port;
    redisContext * _conn;
    //char _user2post_prefix[MAX_RDS_PRFX_LEN];
} redis_service_t;

typedef struct {
    int _user_num;
    uint64_t *_user_ids;
    int *_user_types;
    char *_tag;
    int _start_idx;
    int _req_num;
} redis_get_post_req_t;

typedef struct {
    int _list_num;
    uint64_t _lists[MAX_FOLLOWER_NUM][MAX_RET_POST_NUM];
} redis_get_post_rsp_t;

typedef struct {
    int _user_num;
    uint64_t *_user_ids;
    char *_tag;
} redis_get_post_cnt_req_t;

typedef struct {
    uint64_t _post_id; 
	// the cnt of combination of user_ids and tags.
    int _pair_cnt;

    uint64_t *_user_ids;
    char *_tags[MAX_REF_CNT_PER_POST];
} redis_set_post_req_t; // write post request data

// 创建redis对象，建立连接
redis_service_t *redis_create(char *host, short port);

// 获取post list，需要外部构造查询条件redis_get_post_req_t
// 返回结果保存到rsp中，在外部分配内存
int redis_get_post(redis_service_t *rds, redis_get_post_req_t *req, redis_get_post_rsp_t *rsp);

//added by Radio
int redis_get_post_by_page(redis_service_t *rds, redis_get_post_req_t *req, redis_get_post_rsp_t *rsp);

// 获取post数量
int redis_get_post_cnt(redis_service_t *rds, redis_get_post_cnt_req_t *req);

// 保存post
int redis_set_post(redis_service_t *rds, redis_set_post_req_t *req);

int redis_remove_post(redis_service_t *rds, redis_set_post_req_t *req);

// 释放redis，断开连接
void redis_close(redis_service_t *rds);

#endif

