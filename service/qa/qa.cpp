#include <time.h>
#include <inttypes.h>
#include <string>

#include "log.h"
#include "conf.h"
#include "buffer.h"

#include "hiredis.h"
#include "server_manager.h"
#include "common.h"
#include "interface.h"
#include "common_func.h"

#include "StringUtil.h"

#define QA_INVITE_PAIR_MAX_LEN (20+1+20)

typedef struct {
	char _user2qa_conf_file[MAX_FILE_PATH_LEN];
	char _user2inviteme_conf_file[MAX_FILE_PATH_LEN];
	char _user2answerme_conf_file[MAX_FILE_PATH_LEN];
	char _scenic2qa_conf_file[MAX_FILE_PATH_LEN];
	char _q2a_conf_file[MAX_FILE_PATH_LEN];

    int _recent_range_to_notify;

    char _q2a_uid_conf_file[MAX_FILE_PATH_LEN];
	char _q2awaiting_uid_conf_file[MAX_FILE_PATH_LEN];

	char _q2follow_conf_file[MAX_FILE_PATH_LEN];

	char _push_conf_file[MAX_FILE_PATH_LEN];
} qa_conf_t;

typedef struct {
	svr_mgr_t *_user2qa;
	svr_mgr_t *_user2inviteme;
	svr_mgr_t *_user2answerme;
	svr_mgr_t *_scenic2qa;
	svr_mgr_t *_q2a;

    /* 问题下已回答者的清单 */
    svr_mgr_t *_q2a_uid;
    
    /* 问题下被邀请却尚未回答者的清单 */
    svr_mgr_t *_q2awaiting_uid;
    
    /* 问题的关注者清单.*/
    svr_mgr_t *_q2follow;

	svr_mgr_t *_push;

} qa_service_t;

const int redis_cmd_len = 1024 * 200;
qa_conf_t g_setting;
__thread qa_service_t qa_service = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

static int _build_failed_pack(struct io_buff *buff)
{
	req_pack_t *req = (req_pack_t *)buff->rbuff;
	rsp_pack_t *rsp = (rsp_pack_t *)buff->wbuff;

	// pack response
	rsp->_header.len = sizeof(rsp_pack_t);
	rsp->_header.magic = req->_header.magic;
	rsp->_header.cmd = req->_header.cmd;
	rsp->_header.sequence = req->_header.sequence+1;
	rsp->_header.state = -1;

	return 0;
}

static int _create_redis_conn(char *host, short port, void **conn)
{
	redisContext *c = redisConnect(host, port);
	if (c == NULL) {
		log_txt_err("redisConnect failed, host[%s], port[%d]", host, port);
		return -1;
	}
	if (c->err) {
		log_txt_err("redisConnect failed, host[%s], port[%d], msg:[%s]\n", host, port, c->errstr);
		return -1;
	}
	*conn = c;
	return 0;
}

static int _destroy_redis_conn(void *conn)
{
	redisFree((redisContext *)conn);
	return 0;
}

static int _init_service(svr_mgr_t **svr, char *conf)
{
	if (*svr != NULL) {
		return 0;
	}

	svr_mgr_t *tmp = NULL;
	tmp = sm_init(conf, _create_redis_conn, _destroy_redis_conn);
	if (tmp == NULL) {
		log_txt_err("server manager init failed, conf_file[%s]", conf);
		return -1;
	}

	*svr = tmp;
	return 0;
}

static int _redis_key_del(svr_mgr_t *svr_handler, const char *pfx, uint64_t key)
{
    if (svr_handler == NULL)
	{
        log_txt_err("svr_handler be NULL, pfx:%s", pfx) ;
		return -1;
	}

	svr_group_t *rds = sm_get_svr(svr_handler, key);
	if (rds == NULL) {
		log_txt_err("get %s server failed, key:[%" PRIu64 "]", pfx, key);
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn,
			"DEL %s%" PRIu64 "", pfx, key);
	if (reply == NULL)
	{
		log_txt_err("execute redis command failed: [DEL %s%" PRIu64 "]",
				pfx, key);
		sm_reconnect(svr_handler, rds);
		return -1;
	}
	freeReplyObject(reply);
	return 0;
}

// redis 存字符串
static int _redis_set_string(svr_mgr_t *svr_handler, const char *pfx, uint64_t key, char* buf)
{
    if (svr_handler == NULL)
	{
        log_txt_err("svr_handler be NULL, pfx:%s", pfx) ;
		return -1;
	}
	svr_group_t *rds = sm_get_svr(svr_handler, key);
	if (rds == NULL) {
		log_txt_err("get %s server failed, key:[%" PRIu64 "]", pfx, key);
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn,
			"SET %s%" PRIu64 " %s", pfx, key, buf);
	if (reply == NULL) {
		log_txt_err("execute redis command failed: [SET %s%" PRIu64 "]",
				pfx, key);
		sm_reconnect(svr_handler, rds);
		return -1;
	}
	freeReplyObject(reply);
	return 0;
}

// redis 读字符串
static int _redis_get_string(svr_mgr_t *svr_handler, const char *pfx,
		uint64_t key, char* buf, int buf_len)
{
    if (svr_handler == NULL)
	{
        log_txt_err("svr_handler be NULL, pfx:%s", pfx) ;
		return -1;
	}

	svr_group_t *rds = sm_get_svr(svr_handler, key);
	if (rds == NULL) {
		log_txt_err("get %s server failed, key:[%" PRIu64 "]", pfx, key);
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn,
			"GET %s%" PRIu64 "", pfx, key);
	if (reply == NULL)
	{
		log_txt_err("execute redis command failed: [GET %s%" PRIu64 "]",pfx, key);
		sm_reconnect(svr_handler, rds);
		return -1;
	}
	if (reply->type != REDIS_REPLY_STRING)
	{
		if (reply->type == REDIS_REPLY_ERROR)
		{
			log_txt_err("redis GET string exception: %s", reply->str);
		}
		freeReplyObject(reply);
		return -1;
	}
	if (reply->len > buf_len)
	{
		log_txt_err("redis GET string invalid data");
		freeReplyObject(reply);
		return -1;
	}
	if (reply->len > 0)
	{
		memcpy(buf, reply->str, reply->len);
	}
	freeReplyObject(reply);
	return reply->len;
}

static int _set_redis_list(svr_mgr_t *svr_handler, const char *pfx, uint64_t key, uint64_t value)
{
	if (svr_handler == NULL)
	{
        log_txt_err("svr_handler be NULL, pfx:%s", pfx) ;
		return -1;
	}

	svr_group_t *rds = sm_get_svr(svr_handler, key);
	if (rds == NULL) {
		log_txt_err("get %s server failed, key:[%" PRIu64 "]", pfx, key);
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn,
			"LPUSH %s%" PRIu64 " %" PRIu64 "", pfx, key, value);
	if (reply == NULL)
	{
		log_txt_err("execute redis command failed: [LPUSH %s%" PRIu64 " %" PRIu64 "]", pfx,
				key, value);
		sm_reconnect(svr_handler, rds);
		return -1;
	}

	freeReplyObject(reply);
	return 0;
}

static int _get_redis_list(svr_mgr_t *svr_handler, uint64_t key,
		                   int start_idx, int req_num,
		                   const char *pfx, int ret_size, uint64_t *ret_list)
{
	if (svr_handler == NULL)
	{
        log_txt_err("svr_handler be NULL, pfx:%s", pfx) ;
		return -1;
	}

	svr_group_t *rds = sm_get_svr(svr_handler, key);
	if (rds == NULL) {
		log_txt_err("get %s server failed, key:[%" PRIu64 "]", pfx, key);
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn, "LRANGE %s%" PRIu64 " %d %d",
			pfx, key, start_idx, start_idx + req_num-1);
	if (reply == NULL)
	{
		log_txt_err("execute redis command failed: [LRANGE %s%" PRIu64 " %d %d]",
				pfx, key, start_idx, start_idx + req_num-1);
		sm_reconnect(svr_handler, rds);
		return -1;
	}

	int elementSize = 0;
	for (size_t i = 0; i < reply->elements; i++)
	{
		if (reply->element[i]->type == REDIS_REPLY_INTEGER)
			ret_list[i] = (uint64_t)(reply->element[i]->integer);
		else if (reply->element[i]->type == REDIS_REPLY_STRING)
			sscanf(reply->element[i]->str, "%" PRIu64 "", &(ret_list[i])) ;
		else
		{
			log_txt_err("redis query success, result type incorrect.") ;
			break ;
		}
		if (++elementSize >= ret_size)
		{
			//log_txt_err("return list was full, size:[%d] need_size:[%lu]", ret_size, reply->elements);
			break;
		}
	}
	freeReplyObject(reply);
	return elementSize;
}

/* 从 redis list 中移除一个元素. 最后一个参数默认为true, 移除所有同值元素; 取值false时, 只移除最后一个. */
static int _remove_redis_list(svr_mgr_t *svr_handler, const char *pfx, uint64_t key, uint64_t value, bool all_or_lastone=true)
{
    if (svr_handler == NULL)
	{
        log_txt_err("svr_handler be NULL, pfx:%s", pfx) ;
		return -1;
	}

	svr_group_t *rds = sm_get_svr(svr_handler, key);
	if (rds == NULL) {
		log_txt_err("get %s server failed, key:[%llu]", pfx, (unsigned long long)key);
		return -1;
	}

	redisReply *reply = NULL ;

    if ( all_or_lastone == true )
        reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn, "LREM %s%llu %d %llu", pfx, key, 0, value);
    else
        reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn, "LREM %s%llu %d %llu", pfx, key, -1, value);

	if (reply == NULL)
	{
		log_txt_err("execute redis command faild, command:[LREM %s%llu %llu]", pfx, (unsigned long long)key,
				(unsigned long long)value);
		sm_reconnect(svr_handler, rds);
		return -1;
	}

	freeReplyObject(reply);

	return 0;
}

/* 从 redis list 中 pop 出头部元素, 仅适用于整数类型. */
static int _pop_redis_list(svr_mgr_t *svr_handler, const char *pfx, uint64_t key, uint64_t *result)
{
    if (svr_handler == NULL)
	{
        log_txt_err("svr_handler be NULL, pfx:%s", pfx) ;
		return -1;
	}

	svr_group_t *rds = sm_get_svr(svr_handler, key);
	if (rds == NULL) {
		log_txt_err("get %s server failed, key:[%" PRIu64 "]", pfx, key);
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn,
			"LLEN %s%" PRIu64 "", pfx, key);

	if (reply == NULL) {
		log_txt_err("execute redis command failed, command:[LLEN %s%" PRIu64 "]", pfx, key);
		sm_reconnect(svr_handler, rds);
		return -1;
	}

	int list_size = reply->integer;
	freeReplyObject(reply);
	if ( list_size <= 0 )
		return 0 ;

	reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn, "LPOP %s%" PRIu64, pfx, key);
	if (reply == NULL)
	{
		log_txt_err("execute redis command failed: [LPOP %s%" PRIu64 "]", pfx, key);
		sm_reconnect(svr_handler, rds);
		return -1;
	}
	if ( reply->type == REDIS_REPLY_INTEGER )
		*result = reply->integer;

	else if (reply->type == REDIS_REPLY_STRING)
		sscanf(reply->str, "%" PRIu64 "", result) ;

	else
	{
		log_txt_err("pop result type not integer or string. return 0.") ;

		freeReplyObject(reply);
		return 0;
	}

	freeReplyObject(reply);
	return 1;
}

static int _set_redis_list_string(svr_mgr_t *svr_handler, const char *pfx, uint64_t key, std::string& value)
{
    if (svr_handler == NULL)
	{
        log_txt_err("svr_handler be NULL, pfx:%s", pfx) ;
		return -1;
	}

	svr_group_t *rds = sm_get_svr(svr_handler, key);
	if (rds == NULL) {
		log_txt_err("get %s server failed, key:[%" PRIu64 "]", pfx, key);
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn,
			"LPUSH %s%" PRIu64 " %s", pfx, key, (char *)value.c_str());
	if (reply == NULL)
	{
		log_txt_err("execute redis command failed: [LPUSH %s%" PRIu64 " %s]", pfx, key, (char *)value.c_str());
		sm_reconnect(svr_handler, rds);
		return -1;
	}

	freeReplyObject(reply);
	return 0;
}

static int _remove_redis_list_string(svr_mgr_t *svr_handler, const char *pfx, uint64_t key, std::string& value)
{
    if (svr_handler == NULL)
	{
        log_txt_err("svr_handler be NULL, pfx:%s", pfx) ;
		return -1;
	}

	svr_group_t *rds = sm_get_svr(svr_handler, key);
	if (rds == NULL) {
		log_txt_err("get %s server failed, key:[%" PRIu64 "]", pfx, key);
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn,
			"LREM %s%" PRIu64 " %d %s", pfx, key, 0, (char *)value.c_str());
	if (reply == NULL)
	{
		log_txt_err("execute redis command failed: [LREM %s%" PRIu64 " %s]", pfx, key, (char *)value.c_str());
		sm_reconnect(svr_handler, rds);
		return -1;
	}

	freeReplyObject(reply);
	return 0;
}

static int _get_redis_list_string(svr_mgr_t *svr_handler, uint64_t key,
		                          int start_idx, int req_num,
		                          const char *pfx, int ret_size, char (*ret_list)[QA_INVITE_PAIR_MAX_LEN])
{
    if (svr_handler == NULL)
	{
        log_txt_err("svr_handler be NULL, pfx:%s", pfx) ;
		return -1;
	}

	svr_group_t *rds = sm_get_svr(svr_handler, key);
	if (rds == NULL) {
		log_txt_err("get %s server failed, key:[%" PRIu64 "]", pfx, key);
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn, "LRANGE %s%" PRIu64 " %d %d",
			                                       pfx, key, start_idx, start_idx + req_num-1);
	if (reply == NULL)
	{
		log_txt_err("execute redis command failed: [LRANGE %s%" PRIu64 " %d %d]",
				pfx, key, start_idx, start_idx + req_num-1);
		sm_reconnect(svr_handler, rds);
		return -1;
	}

	int elementSize = 0;
	for (size_t i = 0; i < reply->elements; i++)
	{
		if (reply->element[i]->type != REDIS_REPLY_STRING)
		{
			log_txt_err("redis query success, result type incorrect.") ;
			break ;
		}
        if (reply->element[i]->len > QA_INVITE_PAIR_MAX_LEN )
        {
            log_txt_err("redis GET string invalid data");
            continue ;
        }
        if (reply->element[i]->len > 0)
        {
            memcpy(ret_list[elementSize], reply->element[i]->str, reply->element[i]->len);
        }
		if (++elementSize >= ret_size)
		{
			//log_txt_err("return list was full, size:[%d] need_size:[%lu]", ret_size, reply->elements);
			break;
		}
	}
	freeReplyObject(reply);
	return elementSize;
}

static int _get_redis_list_cnt(svr_mgr_t *svr_handler, const char* pfx, uint64_t key)
{
    if (svr_handler == NULL)
	{
        log_txt_err("svr_handler be NULL, pfx:%s", pfx) ;
		return -1;
	}

	svr_group_t *rds = sm_get_svr(svr_handler, key);
	if (rds == NULL) {
		log_txt_err("get %s server failed, key:[%" PRIu64 "]", pfx, key);
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn,
			"LLEN %s%" PRIu64 "", pfx, key);

	if (reply == NULL) {
		log_txt_err("execute redis command failed, command:[LLEN %s%" PRIu64 "]", pfx, key);
		sm_reconnect(svr_handler, rds);
		return -1;
	}
	int ret = reply->integer;
	freeReplyObject(reply);
	return ret;
}

static int _set_redis_zset(svr_mgr_t *svr_handler, const char *pfx, uint64_t key, uint64_t value)
{
    if (svr_handler == NULL)
	{
        log_txt_err("svr_handler be NULL, pfx:%s", pfx) ;
		return -1;
	}

	svr_group_t *rds = sm_get_svr(svr_handler, key);
	if (rds == NULL) {
		log_txt_err("get %s server failed, key:[%" PRIu64 "]", pfx, key);
		return -1;
	}

    uint64_t stamp = time(NULL) ;
	redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn,
			"ZADD %s%" PRIu64 " %" PRIu64 " %" PRIu64 "", pfx, key, stamp, value);
	if (reply == NULL)
	{
		log_txt_err("execute redis command failed: [ZADD %s%" PRIu64 " %" PRIu64 " %" PRIu64 "]", 
                pfx,key, stamp, value);
		sm_reconnect(svr_handler, rds);
		return -1;
	}

	freeReplyObject(reply);
	return 0;
}

static int _get_redis_zset(svr_mgr_t *svr_handler, uint64_t key,
		int start_idx, int req_num,
		const char *pfx, int ret_size, uint64_t *ret_list)
{
    if (svr_handler == NULL)
	{
        log_txt_err("svr_handler be NULL, pfx:%s", pfx) ;
		return -1;
	}

	svr_group_t *rds = sm_get_svr(svr_handler, key);
	if (rds == NULL) {
		log_txt_err("get %s server failed, key:[%" PRIu64 "]", pfx, key);
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn, "ZREVRANGE %s%" PRIu64 " %d %d",
			pfx, key, start_idx, start_idx + req_num-1);
	if (reply == NULL)
	{
		log_txt_err("execute redis command failed: [ZREVRANGE %s%" PRIu64 " %d %d]",
				pfx, key, start_idx, start_idx + req_num-1);
		sm_reconnect(svr_handler, rds);
		return -1;
	}

	int elementSize = 0;
	for (size_t i = 0; i < reply->elements; i++)
	{
		if (reply->element[i]->type == REDIS_REPLY_INTEGER)
			ret_list[i] = (uint64_t)(reply->element[i]->integer);
		else if (reply->element[i]->type == REDIS_REPLY_STRING)
			sscanf(reply->element[i]->str, "%" PRIu64 "", &(ret_list[i])) ;
		else
		{
			log_txt_err("redis query success, result type incorrect.") ;
			break ;
		}
		if (++elementSize >= ret_size)
		{
			//log_txt_err("return list was full, size:[%d] need_size:[%lu]", ret_size, reply->elements);
			break;
		}
	}
	freeReplyObject(reply);
	return elementSize;
}

static int _get_redis_zset_cnt(svr_mgr_t *svr_handler, const char* pfx, uint64_t key)
{
    if (svr_handler == NULL)
	{
        log_txt_err("svr_handler be NULL, pfx:%s", pfx) ;
		return -1;
	}

	svr_group_t *rds = sm_get_svr(svr_handler, key);
	if (rds == NULL) {
		log_txt_err("get %s server failed, key:[%" PRIu64 "]", pfx, key);
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn,
			"ZCARD %s%" PRIu64 "", pfx, key);

	if (reply == NULL) {
		log_txt_err("execute redis command failed, command:[ZCARD %s%" PRIu64 "]", pfx, key);
		sm_reconnect(svr_handler, rds);
		return -1;
	}
	int ret = reply->integer;
	freeReplyObject(reply);
	return ret;
}

static int _zrank_redis_zset(svr_mgr_t *svr_handler, const char* pfx, uint64_t key, uint64_t value)
{
    if (svr_handler == NULL)
	{
        log_txt_err("svr_handler be NULL, pfx:%s", pfx) ;
		return -1;
	}

	svr_group_t *rds = sm_get_svr(svr_handler, key);
	if (rds == NULL) {
		log_txt_err("get %s server failed, key:[%" PRIu64 "]", pfx, key);
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn,
			"ZREVRANK %s%" PRIu64 " %" PRIu64 "", pfx, key, value);

    /* 当发生错误或者元素不存在时, 本方法的返回值一律为-1. 当元素存在时返回真实索引, 从 0 开始.  */

    /* redis zset 的 zrank 操作在元素不存在时, 返回nil, reply->integer 值为0. 这和元素存在首位   */
    /* 的返回值相同, 会引起混淆. 要区分二者.                                                     */
    
	int ret = -1;
	if (reply != NULL)
    {
        if ( reply->integer > 0 )
            ret = reply->integer ;
        else
        {
            uint64_t first_element = 0 ;
            int expected_size_one = _get_redis_zset(svr_handler, key, 0, 1, pfx, 1, &first_element ) ;
            if ( expected_size_one == 1 && first_element == value )
                ret = 0 ;
        }
	}
	freeReplyObject(reply);
	return ret;
}

static int _remove_redis_zset(svr_mgr_t *svr_handler, const char *pfx, uint64_t key, uint64_t value)
{
    if (svr_handler == NULL)
	{
        log_txt_err("svr_handler be NULL, pfx:%s", pfx) ;
		return -1;
	}

	svr_group_t *rds = sm_get_svr(svr_handler, key);
	if (rds == NULL) {
		log_txt_err("get %s server failed, key:[%llu]", pfx, (unsigned long long)key);
		return -1;
	}

	redisReply *reply = NULL ;

	reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn, "ZREM %s%llu %llu", pfx, key, value);
	if (reply == NULL)
	{
		log_txt_err("execute redis command faild, command:[ZREM %s%llu %llu]", pfx, (unsigned long long)key,
				(unsigned long long)value);
		sm_reconnect(svr_handler, rds);
		return -1;
	}

	freeReplyObject(reply);

	return 0;
}

static int get_svr_pfx(int req_type, svr_mgr_t **svr_handler, const char **pfx)
{
	switch (req_type) {
		case QA_REQ_TYPE_USER:
			*pfx = PFX_USER2QA;
			*svr_handler = qa_service._user2qa;
			break;
		case QA_REQ_TYPE_SCENIC:
			*pfx = PFX_SCENIC2QA;
			*svr_handler = qa_service._scenic2qa;
			break;
		case QA_REQ_TYPE_QUESTION:
			*pfx = PFX_Q2A;
			*svr_handler = qa_service._q2a;
			break;
        case QA_REQ_TYPE_ANSWERME:
            *pfx = PFX_USER2ANSWERME;
            *svr_handler = qa_service._user2answerme;
            break;
        case QA_REQ_TYPE_INVITEME:
            *pfx = PFX_USER2INVITEME;
            *svr_handler = qa_service._user2inviteme;
            break;
        case QA_REQ_TYPE_A_UID_FOR_Q:
            *pfx = PFX_Q2A_UID;
            *svr_handler = qa_service._q2a_uid;
            break;
        case QA_REQ_TYPE_AWAITING_UID_FOR_Q:
            *pfx = PFX_Q2AWAITING_UID;
            *svr_handler = qa_service._q2awaiting_uid;
            break;
        case QA_REQ_TYPE_FOLLOW_FOR_Q:
            *pfx = PFX_Q2FOLLOW;
            *svr_handler = qa_service._q2follow;
            break;
		default:
			log_txt_err("invalid id type: %d", req_type);
			return -1;
	}
	return 0;
}

/* QA 模块, 当有邀请回答或者新答案产生的时候, 向Trans模块推送通知. */

static int _set_push(svr_mgr_t *svr_handler, int push_service_type, uint64_t user_id,
		uint64_t action_uid, uint64_t qa_id, uint64_t amount=0)
{
	if ( push_service_type != PUSH_SERVICE_TYPE_INVITEME && 
         push_service_type != PUSH_SERVICE_TYPE_ANSWERME )
	{
		log_txt_err("push_service_type not valid: %d", push_service_type) ;
		return -1 ;
	}

	/* 获取 push 句柄 */
	svr_group_t *rds = sm_get_svr(svr_handler, user_id);
	if (rds == NULL) {
		log_txt_err("get %s server failed, user_id:[%" PRIu64 "]", PFX_PUSH, user_id);
		return -1;
	}

	char cmd[redis_cmd_len];
	memset(cmd, 0, sizeof(cmd)) ;

    /* 1. 当推送类型为邀请回答的时候, 带上金额字段. 金额为0, 则免费; 否则为付费. */
    /* 2. 拼接 json 串的时候字符较多, 换行时注意行尾和行首, 都添加上双引号.      */

    if (push_service_type == PUSH_SERVICE_TYPE_INVITEME)
    {
        snprintf(cmd, sizeof(cmd), "LPUSH %s {\"type\":%d,\"content\":{\"uid\":\"%" PRIu64 "\","
			                       "\"action_uid\":\"%" PRIu64 "\",\"qa_id\":\"%" PRIu64 "\",\"amount\":\"%" PRIu64 "\"}}",
			                       PFX_PUSH, push_service_type, user_id, action_uid, qa_id, amount);
    }
    else
    {
        snprintf(cmd, sizeof(cmd), "LPUSH %s {\"type\":%d,\"content\":{\"uid\":\"%" PRIu64 "\","
			                       "\"action_uid\":\"%" PRIu64 "\",\"qa_id\":\"%" PRIu64 "\"}}",
			                       PFX_PUSH, push_service_type, user_id, action_uid, qa_id);
    }

	/* 写入 push 消息队列 */
	redisReply *reply = (redisReply*)redisCommand((redisContext*)rds->_cur_conn, cmd);
	if (reply == NULL) {
		log_txt_err("execute redis command faild, command: %s", cmd);
		sm_reconnect(qa_service._push, rds);
		return -1;
	}

	freeReplyObject(reply);

	return 0;
}

static int _set_question(struct io_buff *buff)
{
	if ( _init_service(&qa_service._user2qa, g_setting._user2qa_conf_file) < 0 ||
	     _init_service(&qa_service._scenic2qa, g_setting._scenic2qa_conf_file) < 0 ||
	     _init_service(&qa_service._user2inviteme, g_setting._user2inviteme_conf_file) < 0 ||
		 _init_service(&qa_service._push, g_setting._push_conf_file) < 0)
	{
		return -1 ;
	}

	is2qa_set_question_t *req = (is2qa_set_question_t*)buff->rbuff;
	qa2is_set_question_t *rsp = (qa2is_set_question_t*)buff->wbuff;
	
    rsp->_result = 0;
	rsp->_header.len = sizeof(qa2is_set_question_t);
	rsp->_header.magic = req->_header.magic;
	rsp->_header.cmd = req->_header.cmd;
	rsp->_header.sequence = req->_header.sequence+1;
	rsp->_header.state = 0;

    if (req->_user_id<=0 || req->_question_id<=0 || req->_invited_user_num<0 || req->_scenics_num<0)
    {
        rsp->_result = 1;
        return 0 ;
    }

	if ( _set_redis_zset(qa_service._user2qa, PFX_USER2QA,req->_user_id, req->_question_id) < 0 )
    {
        rsp->_result = 2;
        return 0 ;
    }

    if ( req->_scenics_num > MAX_QA_SCENIC_NUM )
        req->_scenics_num = MAX_QA_SCENIC_NUM ;

	for (int i=0; i<req->_scenics_num; i++)
    {
        if ( _set_redis_zset(qa_service._scenic2qa, PFX_SCENIC2QA, req->_scenics[i], req->_question_id) < 0 )
        {
            rsp->_result = 2;
            return 0 ;
        }
	}

    if ( req->_invited_user_num > MAX_QA_INVITED_NUM )
        req->_invited_user_num = MAX_QA_INVITED_NUM ;
    for (int i=0; i<req->_invited_user_num; i++)
    {
        /* 提问者和被邀请者为同一人时, 跳过. */
        if ( req->_invited_users[i] == req->_user_id )
            continue ;

        std::string value = StringUtil::Uint64ToStr(req->_user_id)+";"+StringUtil::Uint64ToStr(req->_question_id) ;
        if ( _set_redis_list_string(qa_service._user2inviteme, PFX_USER2INVITEME, req->_invited_users[i], value) < 0 )
        {
            rsp->_result = 2;
            return 0 ;
        }
        /* 推送时带上可能的付费金额. 由于初始版本只支持给单人付费, 因此有金额时被邀请的用户数必为1. */
        if (_set_push(qa_service._push, PUSH_SERVICE_TYPE_INVITEME, req->_invited_users[i], req->_user_id, req->_question_id, req->_amount) < 0)
        {
            rsp->_result = 2;
            return 0 ;
        }
        
        /* 将被邀请者列入待回答者的清单中. */
        _set_redis_list(qa_service._q2awaiting_uid, PFX_Q2AWAITING_UID, req->_question_id, req->_invited_users[i]) ;
    }

	return 0;
}

static int _add_follow_q(struct io_buff *buff)
{
	if ( _init_service(&qa_service._q2follow, g_setting._q2follow_conf_file) < 0 )
	{
		return -1 ;
	}

	is2qa_add_follow_q_t *req = (is2qa_add_follow_q_t*)buff->rbuff;
	qa2is_add_follow_q_t *rsp = (qa2is_add_follow_q_t*)buff->wbuff;
	
    rsp->_result = 0;
	rsp->_header.len = sizeof(qa2is_add_follow_q_t);
	rsp->_header.magic = req->_header.magic;
	rsp->_header.cmd = req->_header.cmd;
	rsp->_header.sequence = req->_header.sequence+1;
	rsp->_header.state = 0;

    if (req->_follow_uid<=0 || req->_question_id<=0 || req->_q_uid<=0 || req->_follow_uid==req->_q_uid)
    {
        rsp->_result = 2;
        return 0 ;
    }

	if ( _set_redis_zset(qa_service._q2follow, PFX_Q2FOLLOW,req->_question_id, req->_follow_uid) < 0 )
    {
        rsp->_result = 3;
        return 0 ;
    }

	return 0;
}

static int _del_follow_q(struct io_buff *buff)
{
	if ( _init_service(&qa_service._q2follow, g_setting._q2follow_conf_file) < 0 )
	{
		return -1 ;
	}

	is2qa_del_follow_q_t *req = (is2qa_del_follow_q_t*)buff->rbuff;
	qa2is_del_follow_q_t *rsp = (qa2is_del_follow_q_t*)buff->wbuff;
	
    rsp->_result = 0;
	rsp->_header.len = sizeof(qa2is_del_follow_q_t);
	rsp->_header.magic = req->_header.magic;
	rsp->_header.cmd = req->_header.cmd;
	rsp->_header.sequence = req->_header.sequence+1;
	rsp->_header.state = 0;

    if (req->_follow_uid<=0 || req->_question_id<=0 )
    {
        rsp->_result = 2;
        return 0 ;
    }

	if ( _zrank_redis_zset(qa_service._q2follow, PFX_Q2FOLLOW,req->_question_id, req->_follow_uid) < 0 )
    {
        rsp->_result = 1;
        return 0 ;
    }

	if ( _remove_redis_zset(qa_service._q2follow, PFX_Q2FOLLOW,req->_question_id, req->_follow_uid) < 0 )
    {
        rsp->_result = 3;
        return 0 ;
    }

	return 0;
}

static int _query_follow_q(struct io_buff *buff)
{
	if ( _init_service(&qa_service._q2follow, g_setting._q2follow_conf_file) < 0 )
	{
		return -1 ;
	}

	is2qa_query_follow_q_t *req = (is2qa_query_follow_q_t*)buff->rbuff;
	qa2is_query_follow_q_t *rsp = (qa2is_query_follow_q_t*)buff->wbuff;
	
    rsp->_result = 0;
	rsp->_header.len = sizeof(qa2is_query_follow_q_t);
	rsp->_header.magic = req->_header.magic;
	rsp->_header.cmd = req->_header.cmd;
	rsp->_header.sequence = req->_header.sequence+1;
	rsp->_header.state = 0;

    if (req->_query_uid<=0 || req->_question_id<=0 )
    {
        rsp->_result = 2;
        return 0 ;
    }

	if ( _zrank_redis_zset(qa_service._q2follow, PFX_Q2FOLLOW,req->_question_id, req->_query_uid) < 0 )
    {
        rsp->_result = 1;
        return 0 ;
    }

	return 0;
}

static int _launch_invite(struct io_buff *buff)
{
	if ( _init_service(&qa_service._user2inviteme, g_setting._user2inviteme_conf_file) < 0 ||
		 _init_service(&qa_service._push, g_setting._push_conf_file) < 0 || 
		 _init_service(&qa_service._q2a_uid, g_setting._q2a_uid_conf_file) < 0 || 
		 _init_service(&qa_service._q2awaiting_uid, g_setting._q2awaiting_uid_conf_file) < 0)
	{
		return -1 ;
	}
	
    is2qa_launch_invite_t *req = (is2qa_launch_invite_t*)buff->rbuff;
	qa2is_launch_invite_t *rsp = (qa2is_launch_invite_t*)buff->wbuff;
	
    rsp->_result = 0;
	rsp->_header.len = sizeof(qa2is_launch_invite_t);
	rsp->_header.magic = req->_header.magic;
	rsp->_header.cmd = req->_header.cmd;
	rsp->_header.sequence = req->_header.sequence+1;
	rsp->_header.state = 0;

    /* 参数要合理, 发起者和被邀请者不能同一个人 */
    if (req->_launch_uid<=0 || req->_invited_uid<=0 || req->_question_id<0 || req->_launch_uid == req->_invited_uid)
    {
        rsp->_result = 3;
        return 0 ;
    }

    /* 如果被邀请者近期已答过题了, 则跳过. */
    /* 请注意, 这里采取的策略是只检查最近 recent_range_to_notify 个答者中是否含有当前邀请人. */
    
    uint64_t *recent_uid = new uint64_t[g_setting._recent_range_to_notify] ;
    int ret = _get_redis_list(qa_service._q2a_uid, req->_question_id, 0, g_setting._recent_range_to_notify, 
                              PFX_Q2A_UID, g_setting._recent_range_to_notify, recent_uid) ;
    if ( ret > 0 )
    {
        for (int i=0; i<ret; i++)
        {
            if ( req->_invited_uid == recent_uid[i] )
            {
                rsp->_result = 1;

                delete [] recent_uid ;
                recent_uid = NULL ;

                return 0;
            }
        }
    }

    /* 如果被邀请者近期已被其他人邀请过, 尚未作答, 则不再重复通知, 避免骚扰. */

    ret = _get_redis_list(qa_service._q2awaiting_uid, req->_question_id, 0, g_setting._recent_range_to_notify, 
                          PFX_Q2AWAITING_UID, g_setting._recent_range_to_notify, recent_uid) ;
    if ( ret > 0 )
    {
        for (int i=0; i<ret; i++)
        {
            if ( req->_invited_uid == recent_uid[i] )
            {
                rsp->_result = 2;

                delete [] recent_uid ;
                recent_uid = NULL ;

                return 0;
            }
        }
    }

    delete [] recent_uid ;
    recent_uid = NULL ;

    /* 写入redis 索引中. */
    _set_redis_list(qa_service._q2awaiting_uid, PFX_Q2AWAITING_UID, req->_question_id, req->_invited_uid) ;

    std::string value = StringUtil::Uint64ToStr(req->_launch_uid)+";"+StringUtil::Uint64ToStr(req->_question_id) ;
    if ( _set_redis_list_string(qa_service._user2inviteme, PFX_USER2INVITEME, req->_invited_uid, value) < 0 )
    {
        rsp->_result = 4;
        return 0;
    }

    /* 推送. 第三方邀请回答的时候, 初始版本一律是免费模式. 因此最后一个入参不填, 默认0. */
    if (_set_push(qa_service._push, PUSH_SERVICE_TYPE_INVITEME, req->_invited_uid, req->_launch_uid, req->_question_id) < 0)
    {
        rsp->_result = 4;
        return 0 ;
    }

    return 0 ;
}

static int _set_answer(struct io_buff *buff)
{
	if ( _init_service(&qa_service._user2qa, g_setting._user2qa_conf_file) < 0 ||
	     _init_service(&qa_service._scenic2qa, g_setting._scenic2qa_conf_file) < 0 ||
	     _init_service(&qa_service._q2a, g_setting._q2a_conf_file) < 0 ||
	     _init_service(&qa_service._user2answerme, g_setting._user2answerme_conf_file) < 0 ||
		 _init_service(&qa_service._q2a_uid, g_setting._q2a_uid_conf_file) < 0 || 
		 _init_service(&qa_service._q2awaiting_uid, g_setting._q2awaiting_uid_conf_file) < 0 || 
		 _init_service(&qa_service._q2follow, g_setting._q2follow_conf_file) < 0 || 
		 _init_service(&qa_service._push, g_setting._push_conf_file) < 0)
	{
		return -1 ;
	}

	is2qa_set_answer_t *req = (is2qa_set_answer_t*)buff->rbuff;
	qa2is_set_answer_t *rsp = (qa2is_set_answer_t*)buff->wbuff;
	
    rsp->_result = 0;
	rsp->_header.len = sizeof(qa2is_set_answer_t);
	rsp->_header.magic = req->_header.magic;
	rsp->_header.cmd = req->_header.cmd;
	rsp->_header.sequence = req->_header.sequence+1;
	rsp->_header.state = 0;

    if (req->_q_user_id<=0 || req->_question_id<=0 || req->_a_user_id<=0 || req->_answer_id<=0)
    {
        rsp->_result = 1;
        return 0 ;
    }

	if ( _set_redis_zset(qa_service._user2qa, PFX_USER2QA,req->_a_user_id, req->_answer_id) < 0 )
    {
        rsp->_result = 2;
        return 0 ;
    }

	if ( _set_redis_zset(qa_service._q2a, PFX_Q2A,req->_question_id, req->_answer_id) < 0 )
    {
        rsp->_result = 2;
        return 0 ;
    }

    if ( req->_scenics_num > MAX_QA_SCENIC_NUM )
        req->_scenics_num = MAX_QA_SCENIC_NUM ;

	for (int i=0; i<req->_scenics_num; i++)
    {
        if ( _set_redis_zset(qa_service._scenic2qa, PFX_SCENIC2QA, req->_scenics[i], req->_answer_id) < 0 )
        {
            rsp->_result = 2;
            return 0 ;
        }
	}

    if ( req->_q_user_id != req->_a_user_id )
    {
        if ( _set_redis_list(qa_service._user2answerme, PFX_USER2ANSWERME, req->_q_user_id, req->_answer_id) < 0 )
        {
            rsp->_result = 2;
            return 0 ;
        }
        if (_set_push(qa_service._push, PUSH_SERVICE_TYPE_ANSWERME, req->_q_user_id, req->_a_user_id, req->_answer_id) < 0)
        {
            rsp->_result = 2;
            return 0 ;
        }
    }

    /* 将回答者加入到问题的回答者清单中. */
    _set_redis_list(qa_service._q2a_uid, PFX_Q2A_UID, req->_question_id, req->_a_user_id) ;

    /* 将回答者从该问题的被邀请却尚未回答者的清单中移除. */
    _remove_redis_list(qa_service._q2awaiting_uid, PFX_Q2AWAITING_UID,req->_question_id, req->_a_user_id) ;


    /*
     * 遍历该问题的所有关注者, 将答案写入至每个人的 user2answerme 索引中, 并做推送提醒. 
     *
     * 注意, 当关注者数量很多时, 对每用户都进行 redis 写入和 set_push() 操作, 连接太多, 效率会低下.
     * 将来要优化, 改成 redis 对应的 pipeline 命令, 以及将本次的set_push()方法, 改成pipeline_set_push(). 
     *
     * 如果在qa service 模块中优化后依然同步耗时, 要进一步将这步操作异步转移至 trans 模块去.
     *
     */
    
    int follow_num = _get_redis_zset_cnt(qa_service._q2follow, PFX_Q2FOLLOW, req->_question_id);
    if ( follow_num > 1000 )
        follow_num = 1000 ;

    uint64_t *follow_users = NULL ;
    int follow_user_size = 0 ;

    if ( follow_num > 0 )
    {
        follow_users = new uint64_t[follow_num] ;
        follow_user_size = _get_redis_zset(qa_service._q2follow, req->_question_id, 0, 0, PFX_Q2FOLLOW, follow_num, follow_users ) ;
        if (follow_user_size < 0 )
            follow_user_size = 0 ;
    }
    
    /* 只要回答者不是提问者, 答后默认关注该问题. */
    if ( req->_q_user_id != req->_a_user_id )
        _set_redis_zset(qa_service._q2follow, PFX_Q2FOLLOW, req->_question_id, req->_a_user_id) ;

    if ( follow_num < 0 )
        return 0 ;

    for (int i=0; i<follow_user_size; i++)
    {
        /* 回答者可能之前关注过问题, 不给自己做通知. */

        if ( follow_users[i] == req->_a_user_id )
            continue ;

        _set_redis_list(qa_service._user2answerme, PFX_USER2ANSWERME, follow_users[i], req->_answer_id) ;
        _set_push(qa_service._push, PUSH_SERVICE_TYPE_ANSWERME, follow_users[i], req->_a_user_id, req->_answer_id) ;
    }
    
    if ( follow_users != NULL )
    {
        delete [] follow_users ;
        follow_users = NULL ;
    }

	return 0;
}

static int _del_qa(struct io_buff *buff)
{
	if ( _init_service(&qa_service._user2qa, g_setting._user2qa_conf_file) < 0 ||
	     _init_service(&qa_service._scenic2qa, g_setting._scenic2qa_conf_file) < 0 ||
	     _init_service(&qa_service._user2answerme, g_setting._user2answerme_conf_file) < 0 ||
		 _init_service(&qa_service._q2a_uid, g_setting._q2a_uid_conf_file) < 0 || 
		 _init_service(&qa_service._q2awaiting_uid, g_setting._q2awaiting_uid_conf_file) < 0 || 
		 _init_service(&qa_service._q2follow, g_setting._q2follow_conf_file) < 0 || 
	     _init_service(&qa_service._q2a, g_setting._q2a_conf_file) < 0 )
	{
		return -1 ;
	}

	is2qa_del_qa_t *req = (is2qa_del_qa_t*)buff->rbuff;
	qa2is_del_qa_t *rsp = (qa2is_del_qa_t*)buff->wbuff;
	
    rsp->_result = 0;
	rsp->_header.len = sizeof(qa2is_del_qa_t);
	rsp->_header.magic = req->_header.magic;
	rsp->_header.cmd = req->_header.cmd;
	rsp->_header.sequence = req->_header.sequence+1;
	rsp->_header.state = 0;

    if (req->_qa_user_id<=0 || (req->_qa_type!=0 && req->_qa_type!=1) || req->_question_id<=0)
    {
        rsp->_result = 1;
        return 0 ;
    }

    /* 如果是question的话, 被邀请人数量字段校验. */
    if ( req->_qa_type==0 && (req->_invited_user_num<0 || req->_invited_user_num>MAX_QA_INVITED_NUM) )
    {
        rsp->_result = 1;
        return 0;
    }
    
    /* 如果是answer的话, 问题uid必须存在. */
    if ( req->_qa_type==1 && req->_q_user_id==0)
    {
        rsp->_result = 1;
        return 0;
    }

    uint64_t qa_id = req->_question_id ;
    if ( req->_qa_type == 1)
        qa_id = req->_answer_id ;
	if ( _remove_redis_zset(qa_service._user2qa, PFX_USER2QA, req->_qa_user_id, qa_id) < 0 )
    {
        rsp->_result = 2;
        return 0 ;
    }

    if ( req->_scenics_num > MAX_QA_SCENIC_NUM )
        req->_scenics_num = MAX_QA_SCENIC_NUM ;
	for (int i=0; i<req->_scenics_num; i++)
    {
        if ( _remove_redis_zset(qa_service._scenic2qa, PFX_SCENIC2QA, req->_scenics[i], qa_id) < 0 )
        {
            rsp->_result = 2;
            return 0 ;
        }
	}

    /* 如果是question的话: */

	if ( req->_qa_type == 0 )
    {
        /* 将question id 从 user2inviteme 索引中移除. */
	    if ( req->_invited_user_num > 0 )
        {
            for (int i=0; i<req->_invited_user_num; i++)
            {

                std::string value = StringUtil::Uint64ToStr(req->_qa_user_id) + ";" 
                                  + StringUtil::Uint64ToStr(req->_question_id) ;

                _remove_redis_list_string(qa_service._user2inviteme, PFX_USER2INVITEME, req->_invited_users[i], value) ;
            }
        }

        /* 删除 q2awaiting_uid 索引数据. */
        _redis_key_del(qa_service._q2awaiting_uid, PFX_Q2AWAITING_UID, req->_question_id);
        
        /* 删除 q2follow 索引数据. */
        _redis_key_del(qa_service._q2follow, PFX_Q2FOLLOW, req->_question_id);
    }

    /* 如果是answer的话: */

	if ( req->_qa_type == 1)
    {

        /* 将answer id 从 q2a 索引中移除. */
        if ( _remove_redis_zset(qa_service._q2a, PFX_Q2A,req->_question_id, req->_answer_id) < 0 )
        {
            rsp->_result = 2;
            return 0 ;
        }
    
        /* 从提问者的 user2answerme 中移除. */
        if ( req->_q_user_id != req->_qa_user_id)
        {
            _remove_redis_list(qa_service._user2answerme, PFX_USER2ANSWERME,req->_q_user_id, req->_answer_id) ;
        }

        /* 将回答者的 uid 从 q2a_uid 中移除. */
        _remove_redis_list(qa_service._q2a_uid, PFX_Q2A_UID,req->_question_id, req->_qa_user_id, false) ;
    }
	return 0;
}

static int _attach_detach_scenic(struct io_buff *buff)
{
	if ( _init_service(&qa_service._scenic2qa, g_setting._scenic2qa_conf_file) < 0 ||
	     _init_service(&qa_service._q2a, g_setting._q2a_conf_file) < 0 )
	{
		return -1 ;
	}

	is2qa_attach_detach_scenic_t *req = (is2qa_attach_detach_scenic_t*)buff->rbuff;
	qa2is_attach_detach_scenic_t *rsp = (qa2is_attach_detach_scenic_t*)buff->wbuff;
	
    rsp->_result = 0;
	rsp->_header.len = sizeof(qa2is_attach_detach_scenic_t);
	rsp->_header.magic = req->_header.magic;
	rsp->_header.cmd = req->_header.cmd;
	rsp->_header.sequence = req->_header.sequence+1;
	rsp->_header.state = 0;

    if (req->_qa_id<=0 || (req->_attach_detach_type!=0 && req->_attach_detach_type!=1) || 
        req->_scenic_id<=0 || (req->_qa_type!=0 && req->_qa_type!=1) )
    {
        rsp->_result = 1;
        return 0;
    }
    
    /* 如果是答案的话, 直接把当前答案和旅行地进行关联(或取消). */
    if ( req->_qa_type == 1 )
    {
        /* 当前版本只支持对答案和旅行地的取消关联. */
        if ( req->_attach_detach_type == 1 )
            _remove_redis_zset(qa_service._scenic2qa, PFX_SCENIC2QA, req->_scenic_id, req->_qa_id) ;

        return 0;
    }
    
    /* 如果是问题的话, 要把问题和所有答案, 都与旅行地关联(或取消). */

    int ans_theoretic_num = _get_redis_zset_cnt(qa_service._q2a, PFX_Q2A, req->_qa_id);
    if ( ans_theoretic_num > 1000 )
        ans_theoretic_num = 1000 ;

    uint64_t *qa_range = new uint64_t[ans_theoretic_num+1] ;

    int ans_returned_size = _get_redis_zset(qa_service._q2a, req->_qa_id, 0, 0, PFX_Q2A, ans_theoretic_num, qa_range ) ;
    if (ans_returned_size < 0 )
        ans_returned_size = 0 ;
    
    qa_range[ans_returned_size] = req->_qa_id ;

    for (int i=0; i<ans_returned_size+1; i++)
    {
        if ( req->_attach_detach_type == 0 )
            _set_redis_zset(qa_service._scenic2qa, PFX_SCENIC2QA, req->_scenic_id, qa_range[i]) ;
        else
            _remove_redis_zset(qa_service._scenic2qa, PFX_SCENIC2QA, req->_scenic_id, qa_range[i]) ;
    }

    if ( qa_range != NULL )
    {
        delete [] qa_range ;
        qa_range = NULL ;
    }
	return 0;
}

static int _get_qa(struct io_buff *buff)
{
	if ( _init_service(&qa_service._user2qa, g_setting._user2qa_conf_file) < 0 || 
         _init_service(&qa_service._scenic2qa, g_setting._scenic2qa_conf_file) < 0 ||
         _init_service(&qa_service._q2a, g_setting._q2a_conf_file) < 0 ||
		 _init_service(&qa_service._user2answerme, g_setting._user2answerme_conf_file) < 0 || 
		 _init_service(&qa_service._q2follow, g_setting._q2follow_conf_file) < 0 || 
		 _init_service(&qa_service._q2a_uid, g_setting._q2a_uid_conf_file) < 0 || 
		 _init_service(&qa_service._q2awaiting_uid, g_setting._q2awaiting_uid_conf_file) < 0 )
	{
		return -1;
	}

	is2qa_get_t *req = (is2qa_get_t*)buff->rbuff;
	qa2is_get_t *rsp = (qa2is_get_t*)buff->wbuff;

	const char *pfx = NULL;
	svr_mgr_t *svr_handler = NULL;
	if (get_svr_pfx(req->_req_type, &svr_handler, &pfx) != 0) {
		return -1;
	}

    int ret = 0 ;
    
    /* 本接口不支持对 user2inviteme 索引的查询, 因为其 value 部分为字符串. */
    if ( req->_req_type == QA_REQ_TYPE_INVITEME )
    {
		log_txt_err("QA_REQ_TYPE not supported: %d.", req->_req_type);
        return -1 ;
    }

    /* 本接口支持的7个索引中, 3个 list 结构, 4个 sorted set. */
    else if ( req->_req_type == QA_REQ_TYPE_ANSWERME || 
              req->_req_type == QA_REQ_TYPE_A_UID_FOR_Q || 
              req->_req_type == QA_REQ_TYPE_AWAITING_UID_FOR_Q )
    {
        ret = _get_redis_list(svr_handler, req->_obj_id, req->_start_idx, 
                              req->_req_num, pfx, MAX_RET_QA_NUM, rsp->_objects);
    }
    else
    {
        ret = _get_redis_zset(svr_handler, req->_obj_id, req->_start_idx, 
                              req->_req_num, pfx, MAX_RET_QA_NUM, rsp->_objects);
    }

	if (ret < 0)
    {   
        rsp->_obj_num = 0 ;
		log_txt_err("get redis data failure. %s server, key:[%llu]", pfx, req->_obj_id);
    }
    else
        rsp->_obj_num = ret;

	rsp->_header.len = sizeof(qa2is_get_t)-sizeof(rsp->_objects)+rsp->_obj_num*sizeof(rsp->_objects[0]);
	rsp->_header.magic = req->_header.magic;
	rsp->_header.cmd = req->_header.cmd;
	rsp->_header.sequence = req->_header.sequence+1;
	rsp->_header.state = 0;

	return 0;
}

static int _get_qa_inviteme(struct io_buff *buff)
{
	if ( _init_service(&qa_service._user2inviteme, g_setting._user2inviteme_conf_file) < 0 )
	{
		return -1;
	}

	is2qa_get_inviteme_t *req = (is2qa_get_inviteme_t*)buff->rbuff;
	qa2is_get_inviteme_t *rsp = (qa2is_get_inviteme_t*)buff->wbuff;

    char ret_list[MAX_RET_QA_NUM][QA_INVITE_PAIR_MAX_LEN];

    /* 此处曾经没有对该二维数组进行初始化, 导致获取数据的尾部紊乱. */
    memset(ret_list, 0x00, MAX_RET_QA_NUM*QA_INVITE_PAIR_MAX_LEN);

    int ret = _get_redis_list_string(qa_service._user2inviteme, req->_user_id, req->_start_idx, 
                              req->_req_num, PFX_USER2INVITEME, MAX_RET_QA_NUM, ret_list);

	if (ret < 0)
    {   
        rsp->_qa_num = 0 ;
		log_txt_err("get redis data failure. %s server, key:[%llu]", PFX_USER2INVITEME, req->_user_id);
    }
    else
    {
        rsp->_qa_num = 0;
        for (int i=0; i<ret; i++)
        {
            std::string invite_pair_str(ret_list[i]);
            StrTokenizer tokens(invite_pair_str, ";");
            if (tokens.count_tokens() <= 1)
                continue ;

            rsp->_users_launch_invite[rsp->_qa_num] = StringUtil::StrToUint64(tokens.token(0));
            rsp->_qas[rsp->_qa_num] = StringUtil::StrToUint64(tokens.token(1));
            rsp->_qa_num +=1 ;
        }
    }

	rsp->_header.len = sizeof(qa2is_get_inviteme_t)-sizeof(rsp->_qas)+rsp->_qa_num*sizeof(rsp->_qas[0]);
	rsp->_header.magic = req->_header.magic;
	rsp->_header.cmd = req->_header.cmd;
	rsp->_header.sequence = req->_header.sequence+1;
	rsp->_header.state = 0;

	return 0;
}

static int _get_qa_cnt(struct io_buff *buff)
{
	if ( _init_service(&qa_service._user2qa, g_setting._user2qa_conf_file) < 0 || 
		 _init_service(&qa_service._scenic2qa, g_setting._scenic2qa_conf_file) < 0 || 
		 _init_service(&qa_service._q2a, g_setting._q2a_conf_file) < 0 || 
		 _init_service(&qa_service._user2inviteme, g_setting._user2inviteme_conf_file) < 0 || 
		 _init_service(&qa_service._user2answerme, g_setting._user2answerme_conf_file) < 0 || 
		 _init_service(&qa_service._q2a_uid, g_setting._q2a_uid_conf_file) < 0 || 
		 _init_service(&qa_service._q2follow, g_setting._q2follow_conf_file) < 0 || 
		 _init_service(&qa_service._q2awaiting_uid, g_setting._q2awaiting_uid_conf_file) < 0 )
	{
		return -1;
	}

	is2qa_get_cnt_t *req = (is2qa_get_cnt_t*)buff->rbuff;
	qa2is_get_cnt_t *rsp = (qa2is_get_cnt_t*)buff->wbuff;
	const char *pfx = NULL;
	svr_mgr_t *svr_handler = NULL;
	if (get_svr_pfx(req->_req_type, &svr_handler, &pfx) != 0) {
		return -1;
	}

    int ret= 0 ;
    
    /* user2qa, scenic2qa, q2a, q2follow: sorted set 结构. */
    if ( req->_req_type == QA_REQ_TYPE_USER   || 
         req->_req_type == QA_REQ_TYPE_SCENIC || 
         req->_req_type == QA_REQ_TYPE_QUESTION || 
         req->_req_type == QA_REQ_TYPE_FOLLOW_FOR_Q)

        ret = _get_redis_zset_cnt(svr_handler, pfx, req->_obj_id);
    
    /* user2inviteme, user2answerme, q2a_uid, q2awaiting_uid: list  结构. */
    else
        ret = _get_redis_list_cnt(svr_handler, pfx, req->_obj_id);

	if (ret < 0)
		return -1;

	rsp->_qa_cnt = ret;
	rsp->_header.len = sizeof(qa2is_get_cnt_t);
	rsp->_header.magic = req->_header.magic;
	rsp->_header.cmd = req->_header.cmd;
	rsp->_header.sequence = req->_header.sequence+1;
	rsp->_header.state = 0;
	return 0;
}

/*********************************************************************/
/*                                                                   */
/*                首页的问答栏目 Timeline 拉取.                      */
/*                                                                   */
/*     注意, 和其他Timeline不同的地方在于, 不管拉取多少页, 青驿只展  */
/* 示前 MAX_RET_QA_NUM 条, 后引导至问答广场去浏览.                   */
/*                                                                   */
/* 因此, 对于每一个关注的用户/旅行地, 至多拉取其MAX_RET_QA_NUM       */
/* 条, 然后进行多路归并, 在结果中筛选出前端请求需要的结果区间.       */
/*                                                                   */
/*********************************************************************/

static int _get_qa_timeline(struct io_buff *buff)
{
	if ( _init_service(&qa_service._user2qa, g_setting._user2qa_conf_file) < 0 ||
		 _init_service(&qa_service._scenic2qa, g_setting._scenic2qa_conf_file) < 0)
	{
		return -1;
	}

	is2qa_get_timeline_t *req = (is2qa_get_timeline_t*)buff->rbuff;
	qa2is_get_timeline_t *rsp = (qa2is_get_timeline_t*)buff->wbuff;

	rsp->_header.len = sizeof(qa2is_get_timeline_t) - sizeof(rsp->_qas);
	rsp->_header.magic = req->_header.magic;
	rsp->_header.cmd = req->_header.cmd;
	rsp->_header.sequence = req->_header.sequence+1;
	rsp->_header.state = 0;


	/* 如果请求结果范围的起始位置超过 MAX_RET_QA_NUM, 不拉取, 直接返回.      */

	if ( req->_start_idx > MAX_RET_QA_NUM-1 )
	{
		rsp->_qa_num = 0 ;
		return 0 ;
	}

	/* 如果请求结果范围的终止位置超过 MAX_RET_QA_NUM, 更改请求条数.          */

	if ( req->_start_idx + req->_req_num > MAX_RET_QA_NUM )
		req->_req_num = MAX_RET_QA_NUM - req->_start_idx ;

	/* 读取每个用户(旅行地)的问答信息, 确保读取的用户(旅行地)总数不越界.     */

	if ( req->_user_num > MAX_FOLLOWER_NUM-1 )
		req->_user_num = MAX_FOLLOWER_NUM-1 ;

	/*  开始扫描式读取每个用户(旅行地)下的问答列表, 若非空, 保存相关信息.    */
	/*                                                                       */
	/*  当关注的用户和旅行地数非常多时, 多次去请求Redis数据时, 非常耗时低效. */
	/*  此处采用 BS 服务中类似的做法, 将所有请求命令打包, 以 Pipeline 的     */
	/*  方式请求一次即可, 避免超时情况发生.                                  */

	int _list_num = 0 ;
	uint64_t _lists[MAX_FOLLOWER_NUM][MAX_RET_QA_NUM];
	int _user_types[MAX_FOLLOWER_NUM][MAX_RET_QA_NUM];
	uint64_t _user_ids[MAX_FOLLOWER_NUM][MAX_RET_QA_NUM];

	for (int i=0; i<req->_user_num; i++)
	{
		/*  错误类型       */
		if (req->_user_types[i] != 1 && req->_user_types[i] != 2)
			continue ;

		int zset_req_type = 0 ;

		/*  用户类型: 1    */
		if (req->_user_types[i] == 1)
			zset_req_type = QA_REQ_TYPE_USER ;
		/*  旅行地类型: 2  */
		else
			zset_req_type = QA_REQ_TYPE_SCENIC ;

		const char *pfx = NULL ;
		svr_mgr_t *svr_handler = NULL ;
		if (get_svr_pfx(zset_req_type, &svr_handler, &pfx) != 0)
			continue ;
		svr_group_t *rds = sm_get_svr(svr_handler, req->_user_ids[i]);
		if (rds == NULL) {
			log_txt_err("get %s server failed, key:[%" PRIu64 "]", pfx, req->_user_ids[i]);
			continue ;
		}

		int ret = redisAppendCommand((redisContext *)rds->_cur_conn, "ZREVRANGE %s%" PRIu64 " %d %d",
				pfx, req->_user_ids[i], 0, MAX_RET_QA_NUM-1);
		if (ret != REDIS_OK)
		{
			log_txt_err("redisAppendCmd failed. key:[%s%llu]",pfx, (unsigned long long)req->_user_ids[i]);
			continue ;
		}

		for (int j=0; j<MAX_RET_QA_NUM; j++)
		{
			_user_types[_list_num][j] = req->_user_types[i] ;
			_user_ids[_list_num][j] = req->_user_ids[i] ;
		}
		_list_num ++ ;
	}

	redisReply *reply = NULL;
	for (int i=0; i<_list_num; i++)
	{
		_lists[i][0] = 0 ;

		int list_req_type = 0 ;
		if (_user_types[i][0] == 1)
			list_req_type = QA_REQ_TYPE_USER ;
		else
			list_req_type = QA_REQ_TYPE_SCENIC ;
		const char *pfx = NULL ;
		svr_mgr_t *svr_handler = NULL ;
		if (get_svr_pfx(list_req_type, &svr_handler, &pfx) != 0)
			continue ;
		svr_group_t *rds = sm_get_svr(svr_handler, _user_ids[i][0]);
		if (rds == NULL) {
			log_txt_err("get %s server failed, key:[%" PRIu64 "]", pfx, _user_ids[i][0]);
			continue ;
		}

		if (redisGetReply((redisContext *)rds->_cur_conn, (void **) &reply) != REDIS_OK)
		{
			log_txt_err("redisGetReply failed! pipeline idx[%d]", i);
			continue ;
		}
		int elementSize = 0 ;
		uint64_t tmp_str_2_uint64 = 0 ;
		for (size_t j = 0; j < reply->elements; j++)
		{
			if (reply->element[j]->type == REDIS_REPLY_INTEGER)
				_lists[i][j] = (uint64_t)(reply->element[j]->integer);
			else if (reply->element[j]->type == REDIS_REPLY_STRING)
			{
				sscanf(reply->element[j]->str, "%" PRIu64 "", &tmp_str_2_uint64) ;
				_lists[i][j] = tmp_str_2_uint64 ;
			}
			else
			{
				log_txt_err("redis query success, result type incorrect.") ;
				break ;
			}
			elementSize = j+1 ;
		}
		_lists[i][elementSize] = 0;
		_user_types[i][elementSize] = 0 ;
		_user_ids[i][elementSize] = 0 ;

		freeReplyObject(reply);
	}

	// 多路归并.
	int in_list_x[MAX_RET_QA_NUM] ;
	int in_list_y[MAX_RET_QA_NUM] ;
	int ret_qa_num = multi_merge(_lists, _list_num, rsp->_qas, MAX_RET_QA_NUM,
			                     in_list_x, in_list_y, req->_start_idx, req->_req_num);
	if (ret_qa_num < 0)
	{
		log_txt_err("merge failed!") ;
		return -1;
	}

	for (int j=0; j<ret_qa_num; j++)
	{
		rsp->_user_types[j] = _user_types[in_list_x[j]][in_list_y[j]] ;
		rsp->_user_ids[j] = _user_ids[in_list_x[j]][in_list_y[j]] ;
	}

	rsp->_header.len += sizeof(rsp->_qas[0]) * ret_qa_num;
	rsp->_qa_num = ret_qa_num;

	return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

// 初始化qa, 只会在主线程中调用一次
int qa_init(const char *conf_file)
{
	int ret = 0;
	ret = conf_init(conf_file);
	if (ret < 0) {
		return -1;
	}

	g_setting._user2qa_conf_file[0] = 0x00;
	read_conf_str("QA", "USER_TO_QA_CONF_FILE", g_setting._user2qa_conf_file,
			sizeof(g_setting._user2qa_conf_file), "");
	if (g_setting._user2qa_conf_file[0] == 0x00)
	{
		conf_uninit() ;
		return -2;
	}

	g_setting._user2inviteme_conf_file[0] = 0x00;
	read_conf_str("QA", "USER_TO_INVITEME_CONF_FILE", g_setting._user2inviteme_conf_file,
			sizeof(g_setting._user2inviteme_conf_file), "");
	if (g_setting._user2inviteme_conf_file[0] == 0x00)
	{
		conf_uninit() ;
		return -3;
	}
	
    g_setting._user2answerme_conf_file[0] = 0x00;
	read_conf_str("QA", "USER_TO_ANSWERME_CONF_FILE", g_setting._user2answerme_conf_file,
			sizeof(g_setting._user2answerme_conf_file), "");
	if (g_setting._user2answerme_conf_file[0] == 0x00)
	{
		conf_uninit() ;
		return -3;
	}

	g_setting._scenic2qa_conf_file[0] = 0x00;
	read_conf_str("QA", "SCENIC_TO_QA_CONF_FILE", g_setting._scenic2qa_conf_file,
			sizeof(g_setting._scenic2qa_conf_file), "");
	if (g_setting._scenic2qa_conf_file[0] == 0x00)
	{
		conf_uninit() ;
		return -5;
	}
	
    g_setting._q2a_conf_file[0] = 0x00;
	read_conf_str("QA", "Q_TO_A_CONF_FILE", g_setting._q2a_conf_file,
			sizeof(g_setting._q2a_conf_file), "");
	if (g_setting._q2a_conf_file[0] == 0x00)
	{
		conf_uninit() ;
		return -5;
	}

	g_setting._push_conf_file[0] = 0x00;
	read_conf_str("QA", "PUSH_CONF_FILE", g_setting._push_conf_file,
			sizeof(g_setting._push_conf_file), "");
	if (g_setting._push_conf_file[0] == 0x00)
	{
		conf_uninit() ;
		return -6;
	}

    int recent_range_to_notify = 0 ;
    read_conf_int("QA", "RECENT_RANGE_TO_NOTIFY", &recent_range_to_notify, 7);
    if ( recent_range_to_notify == 0 )
    {
        log_txt_err("RECENT_RANGE_TO_NOTIFY must be config");
        conf_uninit() ;
        return -7;
    }
    if ( recent_range_to_notify <=5 || recent_range_to_notify>=15 )
        recent_range_to_notify = 7 ;

    g_setting._recent_range_to_notify = recent_range_to_notify ;

	g_setting._q2a_uid_conf_file[0] = 0x00;
	read_conf_str("QA", "Q_TO_A_UID_CONF_FILE", g_setting._q2a_uid_conf_file,
			sizeof(g_setting._q2a_uid_conf_file), "");
	if (g_setting._q2a_uid_conf_file[0] == 0x00)
	{
		conf_uninit() ;
		return -8;
	}

	g_setting._q2awaiting_uid_conf_file[0] = 0x00;
	read_conf_str("QA", "Q_TO_AWAITING_UID_CONF_FILE", g_setting._q2awaiting_uid_conf_file,
			sizeof(g_setting._q2awaiting_uid_conf_file), "");
	if (g_setting._q2awaiting_uid_conf_file[0] == 0x00)
	{
		conf_uninit() ;
		return -9;
	}
	
    g_setting._q2follow_conf_file[0] = 0x00;
	read_conf_str("QA", "Q_TO_FOLLOW_CONF_FILE", g_setting._q2follow_conf_file,
			sizeof(g_setting._q2follow_conf_file), "");
	if (g_setting._q2follow_conf_file[0] == 0x00)
	{
		conf_uninit() ;
		return -9;
	}
    
	conf_uninit() ;
	return 0;
}

// 处理问答相关请求
int qa_proc(struct io_buff *buff)
{
	req_pack_t *req = (req_pack_t *)buff->rbuff;

	int ret = 0;

	switch (req->_header.cmd) {
		case CMD_SET_QUESTION:
			ret = _set_question(buff);
			break;
		case CMD_SET_ANSWER:
			ret = _set_answer(buff);
			break;
		case CMD_DEL_QA:
			ret = _del_qa(buff);
			break;
		case CMD_ATTACH_DETACH_SCENIC:
			ret = _attach_detach_scenic(buff);
			break;
		case CMD_GET_QA_TIMELINE:
			ret = _get_qa_timeline(buff);
			break;
        case CMD_GET_QA:
            ret = _get_qa(buff);
            break;
        case CMD_GET_QA_CNT:
            ret = _get_qa_cnt(buff);
            break;
        case CMD_LAUNCH_INVITE:
            ret = _launch_invite(buff);
            break;
        case CMD_GET_QA_INVITEME:
            ret = _get_qa_inviteme(buff);
            break ;
        case CMD_ADD_FOLLOW_Q:
            ret = _add_follow_q(buff);
            break ;
        case CMD_DEL_FOLLOW_Q:
            ret = _del_follow_q(buff);
            break ;
        case CMD_QUERY_FOLLOW_Q:
            ret = _query_follow_q(buff);
            break ;
		default:
			log_txt_err("unknown cmd number:[%d]", req->_header.cmd);
			ret = -1;
			break;
	}

	if (ret < 0) {
		_build_failed_pack(buff);
	}

	return ret;
}

// 释放相关资源，只会在主线程中调用一次
int qa_uninit()
{
	if (qa_service._user2qa)
		sm_uninit(qa_service._user2qa);
	if (qa_service._user2inviteme)
		sm_uninit(qa_service._user2inviteme);
	if (qa_service._user2answerme)
		sm_uninit(qa_service._user2answerme);
	if (qa_service._scenic2qa)
		sm_uninit(qa_service._scenic2qa);
	if (qa_service._q2a)
		sm_uninit(qa_service._q2a);

	if (qa_service._q2a_uid)
		sm_uninit(qa_service._q2a_uid);

	if (qa_service._q2awaiting_uid)
		sm_uninit(qa_service._q2awaiting_uid);

	if (qa_service._q2follow)
		sm_uninit(qa_service._q2follow);

	if (qa_service._push)
		sm_uninit(qa_service._push);

	return 0;
}

#ifdef __cplusplus
}
#endif
