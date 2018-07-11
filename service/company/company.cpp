#include <inttypes.h>
#include <algorithm>
#include <time.h>
#include <set>
#include <string>

#include "server_manager.h"
#include "common.h"
#include "interface.h"
#include "common_func.h"

#include "log.h"
#include "conf.h"
#include "buffer.h"
#include "hiredis.h"

#include "StringUtil.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

#define COMPANY_STR_MAX_LEN (4*(20+1)+2+1+MAX_SCENICS_NUM_PER_COMPANY*(20+1))

typedef struct {
	char _user2company_conf_file[MAX_FILE_PATH_LEN];
	char _scenic2company_conf_file[MAX_FILE_PATH_LEN];
	char _company2company_conf_file[MAX_FILE_PATH_LEN];
	char _company_conf_file[MAX_FILE_PATH_LEN];

	char _push_conf_file[MAX_FILE_PATH_LEN];
	char _company_match_queue_conf_file[MAX_FILE_PATH_LEN];

} company_conf_t;

typedef struct {
	svr_mgr_t *_user2company;   // 用户发布的结伴需求.
	svr_mgr_t *_scenic2company; // 保存旅行地下所有结伴id, 包括过期和撤销的.
	svr_mgr_t *_company2company;
	svr_mgr_t *_company;
	svr_mgr_t *_company_match_queue;

	svr_mgr_t *_push;

} company_service_t;

typedef struct {
	uint64_t user_id;
	uint64_t company_id;
	uint64_t start_date;
	uint64_t end_date;
	int scenic_num;
	uint64_t scenics[MAX_SCENICS_NUM_PER_COMPANY];
} company_info_t;

const int redis_cmd_len = 1024 * 200;
company_conf_t g_setting;
__thread company_service_t company_service = {NULL, NULL, NULL, NULL, NULL, NULL};

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

// redis 存字符串
static int _redis_set_string(svr_mgr_t *svr_handler, const char *pfx, uint64_t key, char* buf)
{
	if (svr_handler == NULL)
		return -1;
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
		return -1;

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

static int _redis_key_del(svr_mgr_t *svr_handler, const char *pfx, uint64_t key)
{
	if (svr_handler == NULL)
		return -1;

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

static int _set_redis_list(svr_mgr_t *svr_handler, const char *pfx, uint64_t key, uint64_t value)
{
	if (svr_handler == NULL)
	{
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

static int _remove_redis_list(svr_mgr_t *svr_handler, const char *pfx, uint64_t key, uint64_t value)
{
	if (svr_handler == NULL) {
		return -1;
	}
	svr_group_t *rds = sm_get_svr(svr_handler, key);
	if (rds == NULL) {
		log_txt_err("get %s server failed, key:[%llu]", pfx, (unsigned long long)key);
		return -1;
	}

	redisReply *reply = NULL ;

	reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn, "LREM %s%llu %d %llu", pfx, key, 0, value);
	if (reply == NULL)
	{
		log_txt_err("execute redis command faild, command:[LREM %s%llu %llu]", pfx, (unsigned long long)key,
				(unsigned long long)value);
		sm_reconnect(svr_handler, rds);
		return -1;
	}

	freeReplyObject(reply);
	
    reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn, "LLEN %s%" PRIu64 "", pfx, key);
    if ( reply!=NULL && (int)(reply->integer)==0)
    {
        redisCommand((redisContext *)rds->_cur_conn, "DEL %s%" PRIu64 "", pfx, key);
    }
	freeReplyObject(reply);

	return 0;
}

/* 从 redis list 中 pop 出头部元素, 仅适用于整数类型. */
static int _pop_redis_list(svr_mgr_t *svr_handler, const char *pfx, uint64_t key, uint64_t *result)
{
	if (svr_handler == NULL)
		return -1;

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

static int _get_redis_list(svr_mgr_t *svr_handler, uint64_t key,
		int start_idx, int req_num,
		const char *pfx, int ret_size, uint64_t *ret_list)
{
	if (svr_handler == NULL)
		return -1;

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
			log_txt_err("return list was full, size:[%d] need_size:[%lu]", ret_size, reply->elements);
			break;
		}
	}
	freeReplyObject(reply);
	return elementSize;
}

static int _get_redis_list_cnt(svr_mgr_t *svr_handler, const char* pfx, uint64_t key)
{
	if (svr_handler == NULL)
		return -1;

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

/* COMPANY 模块, 当有推荐结伴需求的时候, 向Trans模块推送通知. */

static int _set_push(svr_mgr_t *svr_handler, int push_service_type, uint64_t user_id,
		uint64_t action_uid, uint64_t company_id, uint64_t dest_company_id)
{
	if ( push_service_type != PUSH_SERVICE_TYPE_NEW_COMPANY )
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
    
    /* 拼接 json 串时字符较多, 换行时注意行尾和行首, 都添加上双引号. */
	snprintf(cmd, sizeof(cmd), "LPUSH %s {\"type\":%d,\"content\":{\"uid\":\"%" PRIu64 "\","
			"\"action_uid\":\"%" PRIu64 "\",\"company_id\":\"%" PRIu64 "\","
			"\"dest_company_id\":\"%" PRIu64 "\"}}",
			PFX_PUSH, push_service_type, user_id, action_uid, company_id, dest_company_id);

	/* 写入 push 消息队列 */
	redisReply *reply = (redisReply*)redisCommand((redisContext*)rds->_cur_conn, cmd);
	if (reply == NULL) {
		log_txt_err("execute redis command faild, command: %s", cmd);
		sm_reconnect(company_service._push, rds);
		return -1;
	}

	freeReplyObject(reply);

	return 0;
}

static std::string company2string(int company_match_type, company_info_t* company) {

	std::string scenics = StringUtil::Uint64ToStr(company->scenics[0]);
	for ( int i=1; i<company->scenic_num; i++)
		scenics += ","+StringUtil::Uint64ToStr(company->scenics[i]);
	char str[1024];
	memset(str, 0, sizeof(str));

	snprintf(str, sizeof(str)-1, "{\"match_type\":%d,"
			"\"company_id\":%" PRIu64 ","
			"\"user_id\": %" PRIu64 ","
			"\"start_date\": %" PRIu64 ","
			"\"end_date\": %" PRIu64 ","
			"\"scenic_num\": %d,"
			"\"scenics\":[%s]}",
			company_match_type, company->company_id,
			company->user_id, company->start_date,
			company->end_date, company->scenic_num,
			scenics.c_str());
			
	return std::string(str);
}

static int _lpush_company_match(svr_mgr_t *svr_handler, int company_match_type, company_info_t *company)
{
	svr_group_t *rds = sm_get_svr(svr_handler, company->company_id);
	if (rds == NULL) {
		log_txt_err("get %s server failed, company_id:[%" PRIu64 "]", PFX_PUSH, company->company_id);
		return -1;
	}

	std::string json_str = company2string(company_match_type, company);

	redisReply *reply = (redisReply*)redisCommand((redisContext*)rds->_cur_conn,
			"LPUSH %s0 %s ", PFX_COMPANY_MATCH_QUEUE, json_str.c_str());
	if (reply == NULL) {
		log_txt_err("execute redis command faild");
		sm_reconnect(company_service._company_match_queue, rds);
		return -1;
	}

	freeReplyObject(reply);

	return 0;
}

static int save_company_info(company_info_t* company)
{
	if ( company == NULL )
	{
		log_txt_err("Tried to save company, but NULL.");
		return -1;
	}

	std::string company_str = StringUtil::Uint64ToStr(company->user_id) + ";" ;
	company_str += StringUtil::Uint64ToStr(company->company_id) + ";" ;
	company_str += StringUtil::Uint64ToStr(company->start_date) + ";" ;
	company_str += StringUtil::Uint64ToStr(company->end_date) + ";" ;
	company_str += StringUtil::Int64ToStr(company->scenic_num) + ";" ;
	for ( int i=0; i<company->scenic_num; i++)
		company_str += StringUtil::Uint64ToStr(company->scenics[i]) + ";" ;

	int ret = _redis_set_string(company_service._company, PFX_COMPANY, company->company_id, (char *)company_str.c_str());

	return ret;
}

static int get_company_info(uint64_t company_id, company_info_t* company)
{
	char company_str_arr[COMPANY_STR_MAX_LEN] ;
	memset(company_str_arr, 0, sizeof(char)*COMPANY_STR_MAX_LEN) ;
	int ret = _redis_get_string(company_service._company, PFX_COMPANY,
			company_id, company_str_arr, COMPANY_STR_MAX_LEN);

	if ( ret < 0 )
	{
		/* 过期失效. */
		log_txt_info("Tried to get company, but failed.");
		return ret;
	}

	std::string company_str(company_str_arr) ;
	StrTokenizer tokens(company_str, ";") ;
	if ( tokens.count_tokens() <= 5)
	{
		log_txt_err("Company info read out, but inconsistent.");
		return -2 ;
	}
	company->user_id    = StringUtil::StrToUint64(tokens.token(0)) ;
	company->company_id = StringUtil::StrToUint64(tokens.token(1)) ;
	company->start_date = StringUtil::StrToUint64(tokens.token(2)) ;
	company->end_date   = StringUtil::StrToUint64(tokens.token(3)) ;
	company->scenic_num = StringUtil::StrToUint64(tokens.token(4)) ;
	for (int i=0; i<company->scenic_num; i++)
		company->scenics[i] = StringUtil::StrToUint64(tokens.token(5+i)) ;

	return ret;
}

/* 人工为2个结伴取消推荐. */

static int _company_manual_detach(struct io_buff *buff)
{
	if ( _init_service(&company_service._company, g_setting._company_conf_file) < 0 ||
		 _init_service(&company_service._company2company, g_setting._company2company_conf_file) < 0 )
	{
		return -1 ;
	}

	is2company_manual_detach_t *req = (is2company_manual_detach_t*) buff->rbuff;

	company2is_manual_detach_t *rsp = (company2is_manual_detach_t*) buff->wbuff;
	rsp->_result = 0;
	rsp->_header.len = sizeof(company2is_manual_detach_t);
	rsp->_header.magic = req->_header.magic;
	rsp->_header.cmd = req->_header.cmd;
	rsp->_header.sequence = req->_header.sequence+1;
	rsp->_header.state = 0;

	/* 确认2个结伴ID都不为零. */
	if ( req->_lhs_id == 0 || req->_rhs_id == 0 )
	{
		rsp->_result = 1 ;
		return 0 ;
	}

	// 从lhs_id结伴推荐结果中移除结伴rhs_id
	_remove_redis_list(company_service._company2company, PFX_COMPANY2COMPANY, req->_lhs_id, req->_rhs_id);
	
    // 如果为双向移除，则需要从结伴rhs_id的推荐结果中移除lhs_id
	if (req->_is_two_way)
    {
		_remove_redis_list(company_service._company2company, PFX_COMPANY2COMPANY, req->_rhs_id, req->_lhs_id);
	}

	return 0;
}

/* 人工为2个结伴增加推荐. */

static int _company_manual_match(struct io_buff *buff)
{
	if ( _init_service(&company_service._company, g_setting._company_conf_file) < 0 ||
		 _init_service(&company_service._company2company, g_setting._company2company_conf_file) < 0 ||
		 _init_service(&company_service._push, g_setting._push_conf_file) < 0 )
	{
		return -1 ;
	}

	is2company_manual_match_t *req = (is2company_manual_match_t*) buff->rbuff;

	company2is_manual_match_t *rsp = (company2is_manual_match_t*) buff->wbuff;
	rsp->_result = 0;
	rsp->_header.len = sizeof(company2is_manual_match_t);
	rsp->_header.magic = req->_header.magic;
	rsp->_header.cmd = req->_header.cmd;
	rsp->_header.sequence = req->_header.sequence+1;
	rsp->_header.state = 0;

	/* 确认2个结伴ID都不为零. */
	if ( req->_lhs_id == 0 || req->_rhs_id == 0 )
	{
		rsp->_result = 3 ;
		return 0 ;
	}

	company_info_t lhs_company, rhs_company;

	/* 确认是否2个结伴都在有效期内. */

	if ( get_company_info(req->_lhs_id, &lhs_company) <= 0 || get_company_info(req->_rhs_id, &rhs_company) <= 0 )
	{
		rsp->_result = 1;
		return 0 ;
	}

	/* 确认2个结伴, 不来自于同一个用户. */

	if ( lhs_company.user_id == rhs_company.user_id )
	{
		rsp->_result = 3 ;
		return 0 ;
	}

    /* 落地, 并通知. */

	uint64_t companies_in_result[MAX_RET_COMPANY_NUM] ;
	int result_size = _get_redis_list(company_service._company2company, req->_lhs_id, 0,
                                      MAX_RET_COMPANY_NUM, PFX_COMPANY2COMPANY, MAX_RET_COMPANY_NUM, companies_in_result) ;

    bool lhs_to_match = true ;

	if ( result_size > 0 )
	{
		for ( int i=0; i<result_size; i++ )
		{
			if ( req->_rhs_id == companies_in_result[i] )
			{
                lhs_to_match = false ;
                break; 
			}
		}
	}
    if ( lhs_to_match )
    {
        _set_redis_list(company_service._company2company, PFX_COMPANY2COMPANY, req->_lhs_id, req->_rhs_id);

        /* 推荐成功后是否要发送私信通知, 根据存量的推荐结伴数, 来动态决定. */
        bool ready_to_notify = false ;

        if ( result_size <= 1 )
            ready_to_notify = true ;
        else if ( result_size <= 10 )
        {
            if ( result_size % 5 == 0 )
                ready_to_notify = true ;
        }
        else if ( result_size <= 40 )
        {
            if ( result_size % 10 == 0 )
                ready_to_notify = true ;
        }

        if ( ready_to_notify )
        {
            _set_push(company_service._push, PUSH_SERVICE_TYPE_NEW_COMPANY,
                      lhs_company.user_id, rhs_company.user_id,
                      lhs_company.company_id, rhs_company.company_id);
        }
    }


	result_size = _get_redis_list(company_service._company2company, req->_rhs_id, 0, 
                                  MAX_RET_COMPANY_NUM, PFX_COMPANY2COMPANY, MAX_RET_COMPANY_NUM, companies_in_result) ;
    
    bool rhs_to_match = true ;
	
    if ( result_size > 0 )
	{
		for ( int i=0; i<result_size; i++ )
		{
			if ( req->_lhs_id == companies_in_result[i] )
			{
                rhs_to_match = false ;
                break; 
			}
		}
	}
    if ( rhs_to_match == false )
    {
        if (lhs_to_match == false)
            rsp->_result = 2 ;

        return 0 ;
    }

	_set_redis_list(company_service._company2company, PFX_COMPANY2COMPANY, req->_rhs_id, req->_lhs_id);

    bool ready_to_notify = false ;

    if ( result_size <= 1 )
        ready_to_notify = true ;
    else if ( result_size <= 10 )
    {
        if ( result_size % 5 == 0 )
            ready_to_notify = true ;
    }
    else if ( result_size <= 40 )
    {
        if ( result_size % 10 == 0 )
            ready_to_notify = true ;
    }

    if ( ready_to_notify )
    {
        _set_push(company_service._push, PUSH_SERVICE_TYPE_NEW_COMPANY,
                  rhs_company.user_id, lhs_company.user_id, rhs_company.company_id, lhs_company.company_id);
    }

	return 0;
}


static int company_async_match(int type, company_info_t *company)
{
	_lpush_company_match(company_service._company_match_queue, type, company);
	return 0;
}

/*******************************************************************/
/*                                                                 */
/*      人工为已经落地的结伴, 关联景点. (未受保护, 谨慎调用)       */
/*                                                                 */
/* 由于历史遗留问题, 景点页用 redis list 结构来存储结伴, 导致插入  */
/* 结伴的时候, 需要 弹出-比较-插入 的3步非原子操作. 如果此时该景   */
/* 点页下结伴数据发生了变动, 有可能因为没有做同步而产生结伴数据的  */
/* 顺序错乱问题. 虽然是极小概率, 但依然要谨慎调用.                 */
/*                                                                 */
/*******************************************************************/

static int _company_manual_attach_scenics(struct io_buff *buff)
{
	if ( _init_service(&company_service._company, g_setting._company_conf_file) < 0 ||
		 _init_service(&company_service._scenic2company, g_setting._scenic2company_conf_file) < 0 ||
		 _init_service(&company_service._company2company, g_setting._company2company_conf_file) < 0 ||
		 _init_service(&company_service._push, g_setting._push_conf_file) < 0)
	{
		return -1 ;
	}

	is2company_manual_attach_scenics_t *req = (is2company_manual_attach_scenics_t*)buff->rbuff;
	company2is_manual_attach_scenics_t *rsp = (company2is_manual_attach_scenics_t*)buff->wbuff;
	rsp->_result = 0;
	rsp->_header.len = sizeof(company2is_manual_attach_scenics_t);
	rsp->_header.magic = req->_header.magic;
	rsp->_header.cmd = req->_header.cmd;
	rsp->_header.sequence = req->_header.sequence+1;
	rsp->_header.state = 0;

	/* 1. 更新如下数据索引. */

	for (int i=0; i<req->_new_scenics_num; i++)
	{
		/*
		   _set_redis_list(company_service._scenic2company, PFX_SCENIC2COMPANY, req->_new_scenics[i], req->_company_id);
		*/

		/*  为了保证首页 Timeline 拉取归并时, 不会发生错位和遗漏, 需   */
		/*  要严格保证景点页下的结伴, 是按照时间顺序来排列的.          */

		/*  scenic2company 索引本应该采用 sorted set 数据结构, 但由于  */
		/*  历史原因, 错误使用了 list 结构, 导致将结伴关联到景点页下   */
		/*  时需要找到合适的位置插入, 而不能插到链表的头部. 如果依然   */
		/*  坚持, 那么会产生2种结果, 因此必须克服:                     */
		/*                                                             */
		/*  1. 无法去除重复的结伴.                                     */
		/*  2. 当前景点下, 后面有时间更新的结伴时, 可能无法拉取出来.   */

		/*  由于本功能只会被后台的管理人员调用, 绝对是低频操作, 因此   */
		/*  采用最朴素笨拙的办法, 比较元素值代表的时间先后, 择机插入.  */

		uint64_t head_companies[MAX_RET_COMPANY_NUM] ;
		int head_company_num = 0 ;
		uint64_t popped_company = 0 ;

		int pop_num = _pop_redis_list(company_service._scenic2company, PFX_SCENIC2COMPANY,
				                      req->_new_scenics[i], &popped_company) ;

		/*  比较结伴先后时, 和AS BS服务一样假设 ID 包含了时间戳信息.   */

		uint64_t set_company_time = time_from_uuid( req->_company_id ) ;
		while ( pop_num == 1 )
		{
			uint64_t popped_company_time = time_from_uuid( popped_company );
			if ( popped_company_time <= set_company_time )
			{
				_set_redis_list(company_service._scenic2company, PFX_SCENIC2COMPANY,
						req->_new_scenics[i], popped_company);
				break ;
			}
			head_companies[head_company_num] = popped_company ;
			head_company_num ++ ;

			if ( head_company_num == MAX_RET_COMPANY_NUM )
				break ;

			pop_num = _pop_redis_list(company_service._scenic2company, PFX_SCENIC2COMPANY,
					req->_new_scenics[i], &popped_company) ;
		}

		_set_redis_list(company_service._scenic2company, PFX_SCENIC2COMPANY,
				req->_new_scenics[i], req->_company_id);

		for (int j=head_company_num-1; j>=0; j--)
			_set_redis_list(company_service._scenic2company, PFX_SCENIC2COMPANY,
					req->_new_scenics[i], head_companies[j]);
	}

	/* 2. 判断当前结伴是否处于有效期内. 如果已经失效, 则不用做匹配和推荐. */

	company_info_t req_company;
	if ( get_company_info(req->_company_id, &req_company) <= 0 )
	{
		rsp->_result = 1;
		return 0;
	}

	/* 3. 保存最新全量的结伴信息.                                          */
	for ( int i=0; i<req->_new_scenics_num; i++ )
		req_company.scenics[req_company.scenic_num+i] = req->_new_scenics[i];
	req_company.scenic_num += req->_new_scenics_num ;

	save_company_info(&req_company);
	
    /* 4. 针对新增的旅行地, 匹配和通知.                                    */
    req_company.scenic_num = req->_new_scenics_num ;
	for ( int i=0; i<req->_new_scenics_num; i++ )
		req_company.scenics[i] = req->_new_scenics[i];

	company_async_match(COMPANY_MATCH_TYPE_MANUAL, &req_company);

	return 0 ;
}

static int _new_company(struct io_buff *buff)
{
	if ( _init_service(&company_service._company, g_setting._company_conf_file) < 0 ||
	     _init_service(&company_service._user2company, g_setting._user2company_conf_file) < 0 ||
		 _init_service(&company_service._scenic2company, g_setting._scenic2company_conf_file) < 0 ||
		 _init_service(&company_service._company2company, g_setting._company2company_conf_file) < 0 ||
		 _init_service(&company_service._company_match_queue, g_setting._company_match_queue_conf_file) < 0 ||
		 _init_service(&company_service._push, g_setting._push_conf_file) < 0)
	{
		return -1 ;
	}

	is2company_new_t *req = (is2company_new_t*)buff->rbuff;
	company2is_new_t *rsp = (company2is_new_t*)buff->wbuff;
    

	company_info_t company;
	company.user_id = req->_user_id;
	company.company_id = req->_company_id;
	company.start_date = req->_start_date;
	company.end_date = req->_end_date;
	company.scenic_num = req->_scenics_num;

	for (int i = 0; i < company.scenic_num; ++i) {
		company.scenics[i] = req->_scenics[i];
	}

	if (save_company_info(&company) < 0) {
		log_txt_err("save company info to redis failed, company_id:%" PRIu64 "", company.company_id);
		return -1;
	}

    /* 落地2个索引数据. 由于IS层在网络出现错误的情况下会重复发起业务请求, 以及company service 这里 */
    /* 采用了 redis list 数据结构, 因此在落地之前, 需要对可能的重复落地, 进行排除.                 */

	uint64_t existed_ids[10] ;
    bool existed = false ;
	int result_size = 0 ;
    
    result_size = _get_redis_list(company_service._user2company, company.user_id, 0, 
                                  1, PFX_USER2COMPANY, 10, existed_ids) ;
    if ( result_size<=0 || company.company_id != existed_ids[0] )
    {
        _set_redis_list(company_service._user2company, PFX_USER2COMPANY,company.user_id, company.company_id);
    }

    /* 为不是低质量的结伴做2个操作: 1. 将结伴关联到旅行地  2. 进行结伴匹配. */
    if( req -> _low_quality != 1 )
    {
        for (int i=0; i<company.scenic_num; i++)
        {
            result_size = _get_redis_list(company_service._scenic2company, company.scenics[i], 0, 
                                          5, PFX_SCENIC2COMPANY, 10, existed_ids) ;
            existed = false ;
            for (int j=0; j<result_size; j++)
            {
                if (company.company_id == existed_ids[j])
                {
                    existed = true ;
                    break ;
                }
            }
            if (existed == false)
                _set_redis_list(company_service._scenic2company, PFX_SCENIC2COMPANY, 
                                company.scenics[i], company.company_id);
        }
	
        // 将该结伴放入结伴匹配任务队列中
	    company_async_match(COMPANY_MATCH_TYPE_NEW, &company);
    }

	rsp->_header.len = sizeof(company2is_new_t);
	rsp->_header.magic = req->_header.magic;
	rsp->_header.cmd = req->_header.cmd;
	rsp->_header.sequence = req->_header.sequence+1;
	rsp->_header.state = 0;

	return 0;
}


static int _abort_company(struct io_buff *buff)
{
	if ( _init_service(&company_service._company, g_setting._company_conf_file) < 0 ||
		 _init_service(&company_service._user2company, g_setting._user2company_conf_file) < 0 ||
		 _init_service(&company_service._scenic2company, g_setting._scenic2company_conf_file) < 0 ||
		 _init_service(&company_service._company2company, g_setting._company2company_conf_file) < 0 )
	{
		return -1 ;
	}

	is2company_set_t *req = (is2company_set_t*)buff->rbuff;
	company2is_set_t *rsp = (company2is_set_t*)buff->wbuff;

	/*
     * 被动过期和主动撤回:
     *
     * 只需删除用于存放company信息的"company:key"索引.
     */

	if (req->set_flag == COMPANY_SET_FLAG_ABORT || req->set_flag == COMPANY_SET_FLAG_EXPIRED)
	{
        _redis_key_del(company_service._company, PFX_COMPANY, req->_company_id);
	}

    /*
     * 发布人或管理员，主动删除:
     *
     * 还需要将该 company_id 从关联的旅行地和用户下移除.
     */

	else if (req->set_flag == COMPANY_SET_FLAG_DELETED)
	{
        _redis_key_del(company_service._company, PFX_COMPANY, req->_company_id);
        _remove_redis_list(company_service._user2company, PFX_USER2COMPANY, req->_user_id, req->_company_id);

        for (int i=0; i<req->_scenics_num; i++)
        {
		   _remove_redis_list(company_service._scenic2company, PFX_SCENIC2COMPANY, req->_scenics[i], req->_company_id);
        }
	}
	else
	{
		log_txt_err("set company failed, invalid set_flag:%d", req->set_flag);
		return -1;
	}

	rsp->_header.len = sizeof(company2is_set_t);
	rsp->_header.magic = req->_header.magic;
	rsp->_header.cmd = req->_header.cmd;
	rsp->_header.sequence = req->_header.sequence+1;
	rsp->_header.state = 0;
	return 0;
}

static int get_svr_pfx(int req_type, svr_mgr_t **svr_handler, const char **pfx)
{
	switch (req_type) {
		case COMPANY_REQ_TYPE_USER:
			*pfx = PFX_USER2COMPANY;
			*svr_handler = company_service._user2company;
			break;
		case COMPANY_REQ_TYPE_SCENIC:
			*pfx = PFX_SCENIC2COMPANY;
			*svr_handler = company_service._scenic2company;
			break;
		case COMPANY_REQ_TYPE_COMPANY:
			*pfx = PFX_COMPANY2COMPANY;
			*svr_handler = company_service._company2company;
			break;
		default:
			log_txt_err("invalid id type: %d", req_type);
			return -1;
	}
	return 0;
}

static int _get_company(struct io_buff *buff)
{
	if ( _init_service(&company_service._user2company, g_setting._user2company_conf_file) < 0 ||
			_init_service(&company_service._scenic2company, g_setting._scenic2company_conf_file) < 0 ||
			_init_service(&company_service._company2company, g_setting._company2company_conf_file) < 0 )
	{
		return -1;
	}

	is2company_get_t *req = (is2company_get_t*)buff->rbuff;
	company2is_get_t *rsp = (company2is_get_t*)buff->wbuff;

	const char *pfx = NULL;
	svr_mgr_t *svr_handler = NULL;
	if (get_svr_pfx(req->_req_type, &svr_handler, &pfx) != 0) {
		return -1;
	}
	int ret = _get_redis_list(svr_handler, req->_obj_id,
			req->_start_idx, req->_req_num, pfx,
			MAX_RET_COMPANY_NUM, rsp->_companys);
	if (ret < 0) {
		return -1;
	}

	rsp->_company_num = ret;
	rsp->_header.len = sizeof(company2is_get_t)-sizeof(rsp->_companys)+
		rsp->_company_num*sizeof(rsp->_companys[0]);
	rsp->_header.magic = req->_header.magic;
	rsp->_header.cmd = req->_header.cmd;
	rsp->_header.sequence = req->_header.sequence+1;
	rsp->_header.state = 0;

	return 0;
}

static int _get_company_cnt(struct io_buff *buff)
{
	if ( _init_service(&company_service._user2company, g_setting._user2company_conf_file) < 0 ||
			_init_service(&company_service._scenic2company, g_setting._scenic2company_conf_file) < 0 ||
			_init_service(&company_service._company2company, g_setting._company2company_conf_file) < 0 )
	{
		return -1;
	}

	is2company_get_cnt_t *req = (is2company_get_cnt_t*)buff->rbuff;
	company2is_get_cnt_t *rsp = (company2is_get_cnt_t*)buff->wbuff;
	const char *pfx = NULL;
	svr_mgr_t *svr_handler = NULL;
	if (get_svr_pfx(req->_req_type, &svr_handler, &pfx) != 0) {
		return -1;
	}

	int ret = _get_redis_list_cnt(svr_handler, pfx, req->_obj_id);
	if (ret < 0) {
		return -1;
	}

	rsp->_company_cnt = ret;
	rsp->_header.len = sizeof(company2is_get_cnt_t);
	rsp->_header.magic = req->_header.magic;
	rsp->_header.cmd = req->_header.cmd;
	rsp->_header.sequence = req->_header.sequence+1;
	rsp->_header.state = 0;
	return 0;
}

/*********************************************************************/
/*                                                                   */
/*                首页的结伴栏目 Timeline 拉取.                      */
/*                                                                   */
/*     注意, 和其他Timeline不同的地方在于, 不管拉取多少页, 青驿只展  */
/* 示前 MAX_RET_COMPANY_NUM 条, 后引导至结伴广场去浏览.              */
/*                                                                   */
/* 因此, 对于每一个关注的用户/景点, 至多拉取其MAX_RET_COMPANY_NUM    */
/* 条, 然后进行多路归并, 在结果中筛选出前端请求需要的结果区间.       */
/*                                                                   */
/*********************************************************************/

static int _get_company_timeline(struct io_buff *buff)
{
	if ( _init_service(&company_service._user2company, g_setting._user2company_conf_file) < 0 ||
			_init_service(&company_service._scenic2company, g_setting._scenic2company_conf_file) < 0)
	{
		return -1;
	}

	is2company_get_timeline_t *req = (is2company_get_timeline_t*)buff->rbuff;
	company2is_get_timeline_t *rsp = (company2is_get_timeline_t*)buff->wbuff;

	rsp->_header.len = sizeof(company2is_get_timeline_t) - sizeof(rsp->_companies);
	rsp->_header.magic = req->_header.magic;
	rsp->_header.cmd = req->_header.cmd;
	rsp->_header.sequence = req->_header.sequence+1;
	rsp->_header.state = 0;


	/* 如果请求结果范围的起始位置超过 MAX_RET_COMPANY_NUM, 不拉取, 直接返回. */

	if ( req->_start_idx > MAX_RET_COMPANY_NUM-1 )
	{
		rsp->_company_num = 0 ;
		return 0 ;
	}

	/* 如果请求结果范围的终止位置超过 MAX_RET_COMPANY_NUM, 更改请求条数.     */

	if ( req->_start_idx + req->_req_num > MAX_RET_COMPANY_NUM )
		req->_req_num = MAX_RET_COMPANY_NUM - req->_start_idx ;

	/* 读取每个用户(旅行地)下的结伴信息, 确保要读取的用户(旅行地)总数不越界. */

	if ( req->_user_num > MAX_FOLLOWER_NUM-1 )
		req->_user_num = MAX_FOLLOWER_NUM-1 ;

	/*    开始扫描式读取每个用户(旅行地)下的结伴列表, 若非空, 保存相关信息.  */
	/*                                                                       */
	/*    当关注的对象数非常多时, 多次去请求Redis数据时, 非常耗时低效.       */
	/*    此处采用 BS 服务中类似的做法, 将所有请求命令打包, 以 Pipeline 的   */
	/*    方式请求一次即可, 避免超时情况发生.                                */

	int _list_num = 0 ;
	uint64_t _lists[MAX_FOLLOWER_NUM][MAX_RET_COMPANY_NUM];
	int _user_types[MAX_FOLLOWER_NUM][MAX_RET_COMPANY_NUM];
	uint64_t _user_ids[MAX_FOLLOWER_NUM][MAX_RET_COMPANY_NUM];

	for (int i=0; i<req->_user_num; i++)
	{
		/*  错误类型     */
		if (req->_user_types[i] != 1 && req->_user_types[i] != 2)
			continue ;

		int list_req_type = 0 ;

		/*  用户类型: 1  */
		if (req->_user_types[i] == 1)
			list_req_type = COMPANY_REQ_TYPE_USER ;
		/*  景点类型: 2  */
		else
			list_req_type = COMPANY_REQ_TYPE_SCENIC ;

		const char *pfx = NULL ;
		svr_mgr_t *svr_handler = NULL ;
		if (get_svr_pfx(list_req_type, &svr_handler, &pfx) != 0)
			continue ;
		svr_group_t *rds = sm_get_svr(svr_handler, req->_user_ids[i]);
		if (rds == NULL) {
			log_txt_err("get %s server failed, key:[%" PRIu64 "]", pfx, req->_user_ids[i]);
			continue ;
		}
        /*
         * Redis LRANGE 命令的第二个参数, 表示的是结束位置元素的索引号, 而不是表示数量.
         */
		int ret = redisAppendCommand((redisContext *)rds->_cur_conn, "LRANGE %s%" PRIu64 " %d %d",
                                     pfx, req->_user_ids[i], 0, MAX_RET_COMPANY_NUM-1);
		if (ret != REDIS_OK)
		{
			log_txt_err("redisAppendCmd failed. key:[%s%llu]",pfx, (unsigned long long)req->_user_ids[i]);
			continue ;
		}

		for (int j=0; j<MAX_RET_COMPANY_NUM; j++)
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
			list_req_type = COMPANY_REQ_TYPE_USER ;
		else
			list_req_type = COMPANY_REQ_TYPE_SCENIC ;
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
			log_txt_err("redisGetReply failed. pipeline idx[%d]", i);
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

        /*
         * Critical Bug Review, @xiaoming, on 2018-06-14.
         *
         * 使用 Redis 的 LRANGE 命令时, 对第二个参数理解有误, 忘记将数组容量减去1来作为参数,
         * 使得 reply->elements 的结果是: 数组容量+1.
         *
         * 因此下述三行赋值操作时, elementSize 达到了单维数组的容量, 比索引号多1. 由于操作
         * 系统分配二位数组的内存时, 批量分配一块而不区分维度, 因此即使从第一维度的角度看起
         * 来已经越界了, 但是不会报错, 这导致在无 Warning 和 Error 提示情况下, 将第一维度视
         * 角下当前组的下一组第一个元素赋值为了0.
         *
         * 那为什么三组二维数组值赋值错误了, 却一直没有发现呢? 这和青驿的业务逻辑有关. 
         *
         * _list[][] 的值在下一次循环时会重新赋值为正确的, _user_types[][] 和 _user_ids[][]
         * 没有改错的时机. 因此从业务结果来看, 真正的业务元素值是对的, 只是配套它的2个属性
         * 值是错误的, 推迟了这个重要 Bug 的暴露时机.
         *
         */
		_lists[i][elementSize] = 0 ;
		_user_types[i][elementSize] = 0 ;
		_user_ids[i][elementSize] = 0 ;

		freeReplyObject(reply);
	}

	// 多路归并.
	int in_list_x[MAX_RET_COMPANY_NUM] ;
	int in_list_y[MAX_RET_COMPANY_NUM] ;
	int ret_company_num = multi_merge(_lists, _list_num, rsp->_companies, MAX_RET_COMPANY_NUM,
			                          in_list_x, in_list_y, req->_start_idx, req->_req_num);
	if (ret_company_num < 0)
	{
		log_txt_err("merge failed!") ;
		return -1;
	}

	for (int j=0; j<ret_company_num; j++)
	{
		rsp->_user_types[j] = _user_types[in_list_x[j]][in_list_y[j]] ;
		rsp->_user_ids[j] = _user_ids[in_list_x[j]][in_list_y[j]] ;
	}

	rsp->_header.len += sizeof(rsp->_companies[0]) * ret_company_num;
	rsp->_company_num = ret_company_num;

	return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

// 初始化company, 只会在主线程中调用一次
int company_init(const char *conf_file)
{
	int ret = 0;
	ret = conf_init(conf_file);
	if (ret < 0) {
		return -1;
	}

	g_setting._user2company_conf_file[0] = 0x00;
	read_conf_str("COMPANY", "USER_TO_COMPANY_CONF_FILE", g_setting._user2company_conf_file,
			sizeof(g_setting._user2company_conf_file), "");
	if (g_setting._user2company_conf_file[0] == 0x00)
	{
		conf_uninit() ;
		return -3;
	}

	g_setting._scenic2company_conf_file[0] = 0x00;
	read_conf_str("COMPANY", "SCENIC_TO_COMPANY_CONF_FILE", g_setting._scenic2company_conf_file,
			sizeof(g_setting._scenic2company_conf_file), "");
	if (g_setting._scenic2company_conf_file[0] == 0x00)
	{
		conf_uninit() ;
		return -4;
	}

	g_setting._company2company_conf_file[0] = 0x00;
	read_conf_str("COMPANY", "COMPANY_TO_COMPANY_CONF_FILE", g_setting._company2company_conf_file,
			sizeof(g_setting._company2company_conf_file), "");
	if (g_setting._company2company_conf_file[0] == 0x00)
	{
		conf_uninit() ;
		return -5;
	}

	g_setting._push_conf_file[0] = 0x00;
	read_conf_str("COMPANY", "PUSH_CONF_FILE", g_setting._push_conf_file,
			sizeof(g_setting._push_conf_file), "");
	if (g_setting._push_conf_file[0] == 0x00)
	{
		conf_uninit() ;
		return -6;
	}

	g_setting._company_conf_file[0] = 0x00;
	read_conf_str("COMPANY", "COMPANY_CONF_FILE", g_setting._company_conf_file,
			sizeof(g_setting._company_conf_file), "");
	if (g_setting._company_conf_file[0] == 0x00)
	{
		conf_uninit() ;
		return -7;
	}

	g_setting._company_match_queue_conf_file[0] = 0x00;
	read_conf_str("COMPANY", "COMPANY_MATCH_QUEUE_CONF_FILE", g_setting._company_match_queue_conf_file,
			sizeof(g_setting._company_match_queue_conf_file), "");
	if (g_setting._company_match_queue_conf_file[0] == 0x00)
	{
		conf_uninit() ;
		return -8;
	}

	conf_uninit() ;

	return 0;
}

// 处理结伴相关请求
int company_proc(struct io_buff *buff)
{
	req_pack_t *req = (req_pack_t *)buff->rbuff;

	int ret = 0;

	switch (req->_header.cmd) {
		case CMD_GET_COMPANY:
			ret = _get_company(buff);
			break;
		case CMD_GET_COMPANY_CNT:
			ret = _get_company_cnt(buff);
			break;
		case CMD_ABORT_COMPANY:
			ret = _abort_company(buff);
			break;
		case CMD_NEW_COMPANY:
			ret = _new_company(buff);
			break;
		case CMD_GET_COMPANY_TIMELINE:
			ret = _get_company_timeline(buff);
			break;
		case CMD_COMPANY_MANUAL_MATCH:
			ret = _company_manual_match(buff);
			break;
		case CMD_COMPANY_MANUAL_DETACH:
			ret = _company_manual_detach(buff);
			break;
		case CMD_COMPANY_MANUAL_ATTACH_SCENICS:
			ret = _company_manual_attach_scenics(buff);
			break;
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
int company_uninit()
{
	if (company_service._user2company)
		sm_uninit(company_service._user2company);
	if (company_service._scenic2company)
		sm_uninit(company_service._scenic2company);
	if (company_service._company2company)
		sm_uninit(company_service._company2company);
	if (company_service._company)
		sm_uninit(company_service._company);
	if (company_service._company_match_queue)
		sm_uninit(company_service._company_match_queue);

	if (company_service._push)
		sm_uninit(company_service._push);

	return 0;
}

#ifdef __cplusplus
}
#endif
