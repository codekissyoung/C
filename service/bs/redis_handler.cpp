#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>

#include "log.h"
#include "interface.h"
#include "redis_handler.h"
#include "common_func.h"

static redisContext *_redis_connect(char *host, short port)
{
    redisContext *c = redisConnect(host, port);
    if (c == NULL) {
        log_txt_err("redisConnect failed, host[%s], port[%d]", host, port);
        return NULL;
    }
    if (c->err) {
        log_txt_err("redisConnect failed, host[%s], port[%d], msg:[%s]\n", host, port, c->errstr);
        return NULL;
    }

    return c;
}

static int redis_reconnect(redis_service_t *rds)
{
    if (rds->_conn != NULL) {
        redisFree(rds->_conn);
        rds->_conn = NULL;
    }
    rds->_conn = _redis_connect(rds->_host, rds->_port);
    if (rds->_conn == NULL) {
        log_txt_err("Failure to connect redis host:%s, port:%d", rds->_host, rds->_port) ;
        return -1;
    }
    return 0;
}

redis_service_t *redis_create(char *host, short port)
{
    redis_service_t *rds = (redis_service_t *)calloc(1, sizeof(redis_service_t));
    if (rds == NULL) {
        log_txt_err("calloc failed, size[%d]", (int) sizeof(redis_service_t));
        return NULL;
    }

    snprintf(rds->_host, sizeof(rds->_host), "%s", host);
    rds->_port = port;
    rds->_conn = _redis_connect(host, port);
    if (rds->_conn == NULL) {
        free(rds);
        log_txt_err("Failure to connect redis host:%s, port:%d", rds->_host, rds->_port) ;
        return NULL;
    }

    return rds;
}

// 获取redis连接句柄，为了扩展到管理多个redis
// 把获取句柄的单独设置成函数
static redisContext *_redis_get_conn(redis_service_t *rds)
{
    if (rds == NULL)
        return NULL;
    return rds->_conn;
}

int redis_get_post(redis_service_t *rds, redis_get_post_req_t *req, redis_get_post_rsp_t *rsp)
{
    if (rds == NULL || req == NULL || rsp == NULL)
    {
        return -1;
    }

    redisContext *conn = _redis_get_conn(rds);
    if (conn == NULL)
    {
        log_txt_err("get redis handler failed");
        return -1;
    }

    int i = 0;
    int ret = 0;
    int send_ok_num = 0;
    bool is_tag_all = strncmp(req->_tag, "all:", 4) ? false : true;

    /* 使用redis的pipeline，添加查询命令 */
    // 首页timeline全部中仅推送景点下精华文章，保存在redis scenic_best:key中
    for (i = 0; i < req->_user_num; i++)
    {
        if (req->_user_types[i] == 1) { // 用户
            ret = redisAppendCommand(conn, "ZREVRANGE %s%llu %d %d",
                    req->_tag, (unsigned long long)req->_user_ids[i], 0, req->_req_num-1);
            if (ret != REDIS_OK)
            {
                log_txt_err("redisAppendCommand failed!, key:[%s%llu] start:[%d] stop[%d]",
                        req->_tag, (unsigned long long)req->_user_ids[i], 0, req->_req_num-1);

                /* 为了追踪每次收到结果的请求数据, 避免请求数据的索引发生中断, 将 continue 改为break. */
                break ;
                //continue;
            }
        } else if (req->_user_types[i] == 2 && is_tag_all) { //景点
            ret = redisAppendCommand(conn, "ZREVRANGE scenic_best:%llu %d %d",
                    (unsigned long long)req->_user_ids[i], 0, req->_req_num-1);
            if (ret != REDIS_OK)
            {
                log_txt_err("redisAppendCommand failed!, key:[scenic_best:%llu] start:[%d] stop[%d]",
                        (unsigned long long)req->_user_ids[i], 0, req->_req_num-1);

                /* 为了追踪每次收到结果的请求数据, 避免请求数据的索引发生中断, 将 continue 改为break. */
                break ;
                //continue;
            }
        } else {
            continue;
        }
        send_ok_num++;
    }

    /* 等待返回结果 */
    bool redis_failed = false;
    redisReply *reply = NULL;
    rsp->_list_num = 0;
    for (i = 0; i < send_ok_num; i++)
    {
        if (redisGetReply(conn, (void **) &reply) != REDIS_OK)
        {
            log_txt_err("redisGetReply failed! pipeline idx[%d]", i);
            redis_failed = true;
            continue;
        }

        int elementSize = 0 ;
        uint64_t tmp_str_2_uint64 = 0 ;

        for (size_t j = 0; j < reply->elements; j++)
        {
            if (reply->element[j]->type == REDIS_REPLY_INTEGER)
                rsp->_lists[i][j] = (uint64_t)(reply->element[j]->integer);
            else if (reply->element[j]->type == REDIS_REPLY_STRING)
            {
                sscanf(reply->element[j]->str, "%" PRIu64 "", &tmp_str_2_uint64) ;
                rsp->_lists[i][j] = tmp_str_2_uint64 ;
            }
            else
            {
                log_txt_err("redis query success, result type incorrect.") ;
                break ;
            }
            elementSize = j+1 ;
        }
        rsp->_lists[i][elementSize] = 0;

        rsp->_list_num++;
        freeReplyObject(reply);
    }

    if (redis_failed) {
        redis_reconnect(rds);
    }

    return 0;
}

//added by Radio
int redis_get_post_by_page(redis_service_t *rds, redis_get_post_req_t *req, redis_get_post_rsp_t *rsp)
{
    if (rds == NULL || req == NULL || rsp == NULL)
        return -1;

    redisContext *conn = _redis_get_conn(rds);
    if (conn == NULL)
    {
        log_txt_err("get redis handler failed");
        return -1;
    }

    int i = 0;
    int ret = 0;
    int send_ok_num = 0;

    /* 使用redis的pipeline，添加查询命令 */
    for (i = 0; i < req->_user_num; i++)
    {
        ret = redisAppendCommand(conn, "ZREVRANGE %s%llu %d %d",
                req->_tag, (unsigned long long)req->_user_ids[i], req->_start_idx, req->_req_num-1);
        if (ret != REDIS_OK)
        {
            log_txt_err("redisAppendCommand failed!, key:[%s%llu] start:[%d] stop[%d]",
                    req->_tag, (unsigned long long)req->_user_ids[i], 0, req->_req_num-1);

            /* 为了追踪每次收到结果的请求数据, 避免请求数据的索引发生中断, 将 continue 改为break. */
            break ;
            //continue;
        }
        send_ok_num++;
    }

    /* 等待返回结果 */
    bool redis_failed = false;
    redisReply *reply = NULL;
    rsp->_list_num = 0;
    for (i = 0; i < send_ok_num; i++)
    {
        if (redisGetReply(conn, (void **) &reply) != REDIS_OK)
        {
            log_txt_err("redisGetReply failed! pipeline idx[%d]", i);
            redis_failed = true;
            continue;
        }

        int elementSize = 0 ;
        uint64_t tmp_str_2_uint64 = 0 ;

        for (size_t j = 0; j < reply->elements; j++)
        {
            if (reply->element[j]->type == REDIS_REPLY_INTEGER)
                rsp->_lists[i][j] = (uint64_t)(reply->element[j]->integer);
            else if (reply->element[j]->type == REDIS_REPLY_STRING)
            {
                sscanf(reply->element[j]->str, "%" PRIu64 "", &tmp_str_2_uint64) ;
                rsp->_lists[i][j] = tmp_str_2_uint64 ;
            }
            else
            {
                log_txt_err("redis query success, result type incorrect.") ;
                break ;
            }
            elementSize = j+1 ;
        }
        rsp->_lists[i][elementSize] = 0;

        rsp->_list_num++;
        freeReplyObject(reply);
    }

    if (redis_failed) {
        redis_reconnect(rds);
    }

    return 0;
}

int redis_get_post_cnt(redis_service_t *rds, redis_get_post_cnt_req_t *req)
{
    if (rds == NULL || req == NULL)
        return -1;

    redisContext *conn = _redis_get_conn(rds);
    if (conn == NULL) {
        log_txt_err("get redis handler failed");
        return -1;
    }

    int i = 0;
    int ret = 0;
    int send_ok_num = 0;

    /* 使用redis的pipeline，添加查询命令 */
    for (i = 0; i < req->_user_num; i++) {
        ret = redisAppendCommand(conn, "ZCARD %s%llu", req->_tag, (unsigned long long)req->_user_ids[i]);
        if (ret != REDIS_OK) {
            log_txt_err("redisAppendCommand failed!, key:[%s%llu]",
                    req->_tag, (unsigned long long)req->_user_ids[i]);
            continue;
        }

        send_ok_num++;
    }

    /* 等待返回结果 */
    redisReply *reply = NULL;
    bool redis_failed = false;
    int post_cnt = 0;
    for (i = 0; i < send_ok_num; i++) {
        if (redisGetReply(conn, (void **)&reply) != REDIS_OK) {
            log_txt_err("redisGetReply failed! pipeline idx[%d]", i);
            redis_failed = true;
            continue;
        }

        post_cnt += reply->integer;

        freeReplyObject(reply);
    }

    if (redis_failed) {
        redis_reconnect(rds);
    }

    return post_cnt;
}

// 保存post
int redis_set_post(redis_service_t *rds, redis_set_post_req_t *req)
{
    if (rds == NULL || req == NULL)
    {
        log_txt_err("rds or req is NULL.") ;
        return -1;
    }

    redisContext *conn = _redis_get_conn(rds);
    if (conn == NULL) {
        log_txt_err("get redis handler failed");
        return -1;
    }

    bool redis_failed = false;
    redisReply *reply = NULL;
    for (int i = 0; i < req->_pair_cnt; i++)
    {
        uint64_t stamp = time_from_uuid( req->_post_id ) ;

        reply = (redisReply *) redisCommand(conn, "ZADD %s%llu %llu %llu",
                req->_tags[i], (unsigned long long) req->_user_ids[i],
                (unsigned long long) stamp,
                (unsigned long long) req->_post_id
                );

        if (reply == NULL)
        {
            log_txt_err("execute cmd failed, cmd:[ZADD %s%llu %llu %llu]",
                    req->_tags[i], (unsigned long long) req->_user_ids[i],
                    (unsigned long long) stamp,
                    (unsigned long long) req->_post_id
                    );
            redis_failed = true;
            continue;
        }
    }

    if (redis_failed) {
        redis_reconnect(rds);
    }
    return 0;
}

int redis_remove_post(redis_service_t *rds, redis_set_post_req_t *req)
{
    if (rds == NULL || req == NULL) {
        return -1;
    }

    redisContext *conn = _redis_get_conn(rds);
    if (conn == NULL) {
        log_txt_err("get redis handler failed");
        return -1;
    }

    redisReply *reply = NULL;
    bool redis_failed = false;
    for (int i = 0; i < req->_pair_cnt; i++) {
        reply = (redisReply *) redisCommand(conn, "ZREM %s%llu %llu",
                req->_tags[i], (unsigned long long)req->_user_ids[i], (unsigned long long)req->_post_id) ;

        if (reply == NULL) {
            log_txt_err("execute cmd failed, cmd:[ZREM %s%llu %llu]",
                    req->_tags[i], (unsigned long long)req->_user_ids[i], (unsigned long long)req->_post_id);

            redis_failed = true;
            continue;
        }
    }

    if (redis_failed) {
        redis_reconnect(rds);
    }

    return 0;
}

void redis_close(redis_service_t *rds)
{
    if (rds != NULL) {
        if (rds->_conn != NULL) {
            redisFree(rds->_conn);
            rds->_conn = NULL;
        }

        free(rds);
        rds = NULL;
    }
}
