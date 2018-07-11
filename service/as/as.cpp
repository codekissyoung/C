#include <time.h>

#include "buffer.h"
#include "interface.h" 
#include "common.h"
#include "conf.h" 
#include "bs_handler.h"
#include "server_manager.h"
#include "log.h"
#include "hiredis.h"
#include "ul_dictmatch.h"
#include "common_func.h"

#include "StringUtil.h"

/// 配置信息
typedef struct {
    int _bs_group_num;
    bs_group_conf_t *_bs_info;
    int _get_post_max_time;
    int _set_post_max_time;

    /* 景点keyword路径，post中包含这些keyword的时候，
     * post会自动关联到景点 */
    char _key_word_path[MAX_FILE_PATH_LEN];

    /* 景点keyword词典，多模匹配词典 */
    dm_dict_t *_key_word_dict;
    
    /* @我的文章数据服务配置 */
    char _atme_conf_file[MAX_FILE_PATH_LEN];
    
    /* 收藏的文章列表 */
    char _enshrine_conf_file[MAX_FILE_PATH_LEN];
    
    char _post2user_conf_file[MAX_FILE_PATH_LEN] ;
    char _post2paiduser_conf_file[MAX_FILE_PATH_LEN] ;
    char _post2at_conf_file[MAX_FILE_PATH_LEN] ;

    /* 推送服务配置文件 */
    char _push_conf_file[MAX_FILE_PATH_LEN];
} as_conf_t;

typedef struct {
    bs_service_t *_bsc;
    svr_mgr_t *_atme;
    svr_mgr_t *_enshrine;
    svr_mgr_t *_post2user;
    svr_mgr_t *_post2paiduser;
    svr_mgr_t *_post2at;
    svr_mgr_t *_push;
} as_service_t; 

static as_conf_t g_setting;
const int redis_cmd_len = 1024 * 200;

/// 线程内部全局变量
__thread as_service_t as_service = {NULL, NULL, NULL, NULL, NULL, NULL,NULL};
    
static dm_dict_t *_load_key_word(char *path)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        log_txt_err("open file failed! file:[%s]", path);
        return NULL;
    }

    int line_cnt = 0;
    char line[MAX_NAME_LEN*2];

    while (fgets(line, sizeof(line), fp)) {
        line_cnt++;
    }
    fseek(fp, 0, SEEK_SET);

    dm_dict_t * dict = dm_dict_create(line_cnt);
    if (dict == NULL) {
        log_txt_err("dm_dict_create failed! size:[%d]", line_cnt);
        fclose(fp);
        return NULL;
    }

    while (fgets(line, sizeof(line), fp)) {
        char *ptr = strchr(line, '\t');
        if (ptr == NULL) {
            log_txt_err("line format error, line:[%s]", line);
            continue;
        }
        *ptr++ = 0x00;

        dm_lemma_t lm;
        lm.pstr = line;
        lm.len  = strlen(line);
        lm.prop = strtoll(ptr, NULL, 10);

        if (dm_add_lemma(dict, &lm, DM_CHARSET_UTF8) < 0) {
            log_txt_err("add keyword to dmdict failed, keyword:[%s] value:[%s]", line, ptr);
            continue;
        }
    }

    fclose(fp);
    return dict;
}

static int _read_conf(const char *conf_file)
{
    int ret = 0;
    ret = conf_init(conf_file);
    if (ret < 0) {
        log_txt_err("initialize conf failed! conf[%s]", conf_file);
        return -1;
    }

    int bs_group_num = 0;

    read_conf_int("AS", "SET_POST_MAX_TIME", &g_setting._set_post_max_time, 200);
    read_conf_int("AS", "GET_POST_MAX_TIME", &g_setting._get_post_max_time, 200);

    read_conf_str("AS", "SCENIC_KEY_WORD_DICT", g_setting._key_word_path,
                  sizeof(g_setting._key_word_path), "");
    if (g_setting._key_word_path[0] == 0x00) {
        log_txt_err("SCENIC_KEY_WORD_DICT must be config");
        conf_uninit();
        return -1;
    }

    g_setting._key_word_dict = _load_key_word(g_setting._key_word_path);
    if (g_setting._key_word_dict == NULL) {
        log_txt_err("load_key_word failed, path:[%s]", g_setting._key_word_path);
        conf_uninit();
        return -1;
    }
    
    read_conf_str("AS", "ATME_CONF_FILE", g_setting._atme_conf_file,
                  sizeof(g_setting._atme_conf_file), "");

    read_conf_str("AS", "ENSHRINE_CONF_FILE", g_setting._enshrine_conf_file,
                  sizeof(g_setting._enshrine_conf_file), "");
    
    read_conf_str("AS", "POST_TO_USER_CONF_FILE", g_setting._post2user_conf_file,
                  sizeof(g_setting._post2user_conf_file), "");
    
    read_conf_str("AS", "POST_TO_PAIDUSER_CONF_FILE", g_setting._post2paiduser_conf_file,
                  sizeof(g_setting._post2paiduser_conf_file), "");

    read_conf_str("AS", "POST_TO_AT_CONF_FILE", g_setting._post2at_conf_file,
                  sizeof(g_setting._post2at_conf_file), "");

    read_conf_str("AS", "PUSH_CONF_FILE", g_setting._push_conf_file,
                  sizeof(g_setting._push_conf_file), "");

    read_conf_int("AS", "BS_GROUP_NUM", &bs_group_num, 0);
    if (bs_group_num == 0) {
        log_txt_err("BS_GROUP_NUM must be config");
            
        conf_uninit();
        return -1;
    }
    
    g_setting._bs_group_num = bs_group_num;
    g_setting._bs_info = (bs_group_conf_t *)calloc(bs_group_num, sizeof(bs_group_conf_t));
    for (int i = 0; i < bs_group_num; i++) {
        char section[128];
        int bs_num = 0;
        bs_group_conf_t * bs_info = g_setting._bs_info;

        snprintf(section, sizeof(section), "BS_GROUP_%d", i);

        read_conf_int(section, "BS_NUM", &bs_num, 0);
        if (bs_num == 0) {
            log_txt_err("BS_NUM must be config");
            
            conf_uninit();
            return -1;
        }

        bs_info[i]._bak_num = bs_num;

        for (int j = 0; j < bs_num; j++) {
            char key[128];
            snprintf(key, sizeof(key), "BS_HOST_%d", j);
            read_conf_str(section, key, bs_info[i]._conf[j]._host, 
                    sizeof(bs_info[i]._conf[j]._host), "");

            snprintf(key, sizeof(key), "BS_PORT_%d", j);
            int _port ;
            read_conf_int(section, key, &_port, 0);
            bs_info[i]._conf[j]._port = _port ;

            /* check */
            if (bs_info[i]._conf[j]._host[0] == 0x00 || bs_info[i]._conf[j]._port == 0) {
                log_txt_err("read [%s]->[BS_HOST_%d] or [%s]->[BS_PORT_%d] failed", 
                        section, j, section, j);

                conf_uninit();
                return -1;
            }
        }
    }

    conf_uninit();

    return 0;   
}

static int _get_redis_list(svr_mgr_t *svr_handler, uint64_t key, 
                           int start_idx, int req_num,
                           const char *pfx, int ret_size, uint64_t *ret_list) 
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
    reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn, "LRANGE %s%llu %d %d", 
                pfx, key, start_idx, start_idx + req_num-1);
    if (reply == NULL)
    {
        log_txt_err("execute redis command failed, command:[LRANGE %s%llu %d %d]", 
                    pfx, (unsigned long long)key, start_idx, start_idx + req_num-1);
        sm_reconnect(svr_handler, rds);
        return -1;
    }

    int elementSize = 0;
    for (size_t i = 0; i < reply->elements; i++)
    {
        if (reply->element[i]->type == REDIS_REPLY_INTEGER)
            ret_list[i] = (uint64_t)(reply->element[i]->integer);
        else if (reply->element[i]->type == REDIS_REPLY_STRING)
            sscanf(reply->element[i]->str, "%llu", (unsigned long long *)&(ret_list[i])) ;
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

static int _set_redis_list(svr_mgr_t *svr_handler, const char *pfx, uint64_t key, uint64_t value) 
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

    reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn, "LPUSH %s%llu %llu", pfx, key, value);
    if (reply == NULL)
    {
        log_txt_err("execute redis command faild, command:[LPUSH %s%llu %llu]", pfx, 
                    (unsigned long long)key, (unsigned long long)value);
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

    return 0;
}

static int _set_redis_zset(svr_mgr_t *svr_handler, const char *pfx, uint64_t key, uint64_t value)
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

static int _remove_redis_zset(svr_mgr_t *svr_handler, const char *pfx, uint64_t key, uint64_t value)
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

static int _set_redis_zset_string(svr_mgr_t *svr_handler, const char *pfx, uint64_t key, std::string& value)
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

    uint64_t stamp = time(NULL) ;
	redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn,
			"ZADD %s%" PRIu64 " %" PRIu64 " %s", pfx, key, stamp, (char *)value.c_str());
	if (reply == NULL)
	{
		log_txt_err("execute redis command failed: [ZADD %s%" PRIu64 " %s]", pfx, key, (char *)value.c_str());
		sm_reconnect(svr_handler, rds);
		return -1;
	}

	freeReplyObject(reply);
	return 0;
}

static int _remove_redis_zset_string(svr_mgr_t *svr_handler, const char *pfx, uint64_t key, std::string& value)
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
			            "ZREM %s%" PRIu64 " %s", pfx, key, (char *)value.c_str());
	if (reply == NULL)
	{
		log_txt_err("execute redis command failed: [ZREM %s%" PRIu64 " %s]", pfx, key, (char *)value.c_str());
		sm_reconnect(svr_handler, rds);
		return -1;
	}

	freeReplyObject(reply);
	return 0;
}

static int _get_redis_zset_hybrid(svr_mgr_t *svr_handler, uint64_t key,
		                          int start_idx, int req_num,
		                          const char *pfx, int ret_size, int *ret_type_list, uint64_t *ret_value_list)
{
	if (svr_handler == NULL)
		return -1;

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
        /* 当值类型为整数时, 对象肯定是post.  */
		if (reply->element[i]->type == REDIS_REPLY_INTEGER)
		{
            ret_type_list[i] = 0;
            ret_value_list[i] = (uint64_t)(reply->element[i]->integer);
        }
		else if (reply->element[i]->type == REDIS_REPLY_STRING)
        {
            std::string tmpResult(reply->element[i]->str) ;
            StrTokenizer tokens(tmpResult, ";");
            if (tokens.count_tokens() == 0 || tokens.count_tokens() >=3)
            {
                log_txt_err("redis query success, result type incorrect.") ;
                break;
            }
            else if ( tokens.count_tokens() == 1 )
            {
                ret_type_list[i] = 0;
                ret_value_list[i] = StringUtil::StrToUint64(tokens.token(0));
            }
            /* 当值为用分号拼接的字符串时, 对象肯定是问题 答案 产品等对象. */
            else
            {
                ret_type_list[i] = StringUtil::StrToUint64(tokens.token(0));
                ret_value_list[i] = StringUtil::StrToUint64(tokens.token(1));
            }
        }
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

static int _get_as_item_cnt(struct io_buff *buff, svr_mgr_t *svr_handler, const char *pfx)
{
    is2as_get_as_item_cnt_t *req = (is2as_get_as_item_cnt_t *) buff->rbuff;
    as2is_get_as_item_cnt_t *rsp = (as2is_get_as_item_cnt_t *) buff->wbuff;

    svr_group_t *rds = sm_get_svr(svr_handler, req->_user_id);
    if ( rds == NULL )
    {
        log_txt_err("get %s server failed, input user_id:[%llu]", pfx, req->_user_id);
        return -1 ;
    }
    
    redisReply *reply = NULL ;

    /* AS 负责管理的数据服务中, 除了"收藏"采用了 sorted set 结构外, 其他采用的都是 list. */

    if ( strcmp(pfx, PFX_ENSHRINE) == 0 )
    {
        reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn, "ZCARD %s%llu", pfx, req->_user_id) ;
    }
    else
    {
        reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn, "LLEN %s%llu", pfx, req->_user_id) ;
    }

    if (reply == NULL)
    {
        log_txt_err("execute redis cmd failure, cmd:[ZCARD/LLEN %s%llu]", pfx, (unsigned long long)req->_user_id);
        sm_reconnect(svr_handler, rds);
        return -1;
    }

    rsp->_header.len = sizeof(as2is_get_as_item_cnt_t);
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = 0;
    rsp->_item_num = reply->integer;

    return 0;
}

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
    redisFree( (redisContext*) conn);
    return 0;
}

static int _init_server_manager(svr_mgr_t **svr, char *conf)
{
    if (*svr != NULL) {
        return -1;
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

static int _init_atme()
{
    if (as_service._atme != NULL) {
        return -1;
    }

    return _init_server_manager(&as_service._atme, g_setting._atme_conf_file);
}

static int _init_enshrine()
{
    if (as_service._enshrine != NULL) {
        return -1;
    }

    return _init_server_manager(&as_service._enshrine, g_setting._enshrine_conf_file);
}

static int _init_post2user()
{
    if (as_service._post2user != NULL) {
        return -1;
    }

    return _init_server_manager(&as_service._post2user, g_setting._post2user_conf_file);
}

static int _init_post2paiduser()
{
    if (as_service._post2paiduser != NULL) {
        return -1;
    }

    return _init_server_manager(&as_service._post2paiduser, g_setting._post2paiduser_conf_file);
}

static int _init_post2at()
{
    if (as_service._post2at != NULL) {
        return -1;
    }

    return _init_server_manager(&as_service._post2at, g_setting._post2at_conf_file);
}

static int _init_push()
{
    if (as_service._push != NULL) {
        return -1;
    }

    return _init_server_manager(&as_service._push, g_setting._push_conf_file);
}

/* AS 模块中需要负责2种类型消息的推送服务: 新增timeline, at我的主贴.              */

/* 在新增 timeline 中需要 post_id, 在at我的主贴中需要 action_uid 以使用提醒设置   */
/* 因此第4个入参使用通用的obj_id来泛指.                                           */

static int _set_push(svr_mgr_t *svr_handler, int push_service_type, uint64_t user_id, uint64_t obj_id=0) 
{
    /* 获取redis句柄 */
    svr_group_t *rds = sm_get_svr(svr_handler, user_id);
    if (rds == NULL) {
        log_txt_err("get %s server failed, user_id:[%llu]", PFX_PUSH, (unsigned long long)user_id);
        return -1;
    }

    if ( push_service_type != PUSH_SERVICE_TYPE_ATME && 
         push_service_type != PUSH_SERVICE_TYPE_TIMELINE) {

        log_txt_err("push_service_type not valid: %d", push_service_type) ;
        return -1 ;
    }

    if ( push_service_type == PUSH_SERVICE_TYPE_TIMELINE && obj_id <= 0) {
        log_txt_err("push_service_type is timeline, but post_id is 0.") ;
        return -1 ;
    }
    
    if ( push_service_type == PUSH_SERVICE_TYPE_ATME && obj_id <= 0) {
        log_txt_err("push_service_type is atme, but action_uid is 0.") ;
        return -1 ;
    }

    char cmd[redis_cmd_len];
    memset(cmd, 0, sizeof(cmd)) ;
    char *ptr = cmd;
    if (push_service_type == PUSH_SERVICE_TYPE_ATME)
    {
        snprintf(ptr, sizeof(cmd), "LPUSH %s {\"type\":%d,\"content\":{\"uid\":\"%llu\",\"action_uid\":\"%llu\"}}", 
                 PFX_PUSH, push_service_type, (unsigned long long)user_id, (unsigned long long)obj_id);
    }
    else
    {
        snprintf(ptr, sizeof(cmd), "LPUSH %s {\"type\":%d,\"content\":{\"pid\":\"%llu\",\"uid\":\"%llu\"}}", 
                 PFX_PUSH, push_service_type, (unsigned long long)obj_id, (unsigned long long)user_id);
    }

    /* 写入 push 消息队列 */
    redisReply *reply = (redisReply*)redisCommand((redisContext*)rds->_cur_conn, cmd);
    if (reply == NULL) {
        log_txt_err("execute redis command faild, command: %s", cmd);
        sm_reconnect(svr_handler, rds);
        return -1;
    }

    freeReplyObject(reply);

    return 0;
}

static int _at_user(svr_mgr_t *svr_handler, uint64_t post_id, uint64_t user_id)
{
    /* 获取redis句柄 */
    svr_group_t *rds = sm_get_svr(svr_handler, user_id);
    if (rds == NULL) {
        log_txt_err("get %s server failed, user_id:[%llu]", PFX_ATME, (unsigned long long)user_id);
        return -1;
    }

    /* 修改 atme 索引表. */
    redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn, "LPUSH %s%llu %llu", PFX_ATME, user_id, post_id);
    if (reply == NULL)
    {
        log_txt_err("execute redis command failed, command:[LPUSH %s%llu]", 
                    PFX_ATME, (unsigned long long)user_id);
        sm_reconnect(svr_handler, rds);
        return -1;
    }

    freeReplyObject(reply);
    return 0;
}

static int _at_users(uint64_t user_id, uint64_t post_id, int at_user_num, uint64_t *users)
{
    if (as_service._atme == NULL && _init_atme() < 0) {
        return -1;
    }
    
    if (as_service._push == NULL && _init_push() < 0) {
        return -1;
    }

    for (int i = 0; i < at_user_num; i++) {
        if (_at_user(as_service._atme, post_id, users[i]) < 0) {
            log_txt_err("at user failed! user_id:[%llu], post_id:[%llu]", (unsigned long long)users[i], 
                        (unsigned long long)post_id);
        }
        
        if (_set_push(as_service._push, PUSH_SERVICE_TYPE_ATME, users[i], user_id) < 0) {
            log_txt_err("push failed! user_id:[%llu], post_id:[%llu]", (unsigned long long)users[i], 
                        (unsigned long long)post_id);
        }
    }

    return 0;
}


static int _proc_get_post(struct io_buff *buff)
{
    int ret = 0;
    is2as_get_post_t *req = (is2as_get_post_t *) buff->rbuff;
    as2is_get_post_t *rsp = (as2is_get_post_t *) buff->wbuff;

    bs_get_post_req_t bs_req;
    bs_req._user_num = req->_user_num;
    if ( bs_req._user_num > MAX_FOLLOWER_NUM-1 )
        bs_req._user_num = MAX_FOLLOWER_NUM - 1 ;

    bs_req._user_types = req->_user_types;
    bs_req._user_ids = req->_user_ids;
    bs_req._tag = req->_tag;
    
    /*
    bs_req._start_idx = req->_start_idx;
    bs_req._req_num = req->_req_num;
    */

    /* 向BS发起请求时, 请求从第1篇到最多可能的那一篇. */
    bs_req._start_idx = 0 ;
    bs_req._req_num = req->_start_idx + req->_req_num ;

    bs_get_post_rsp_t bs_rsp;
    ret = bs_get_post(as_service._bsc, &bs_req, &bs_rsp, g_setting._get_post_max_time);
    if (ret < 0) {
        log_txt_err("bs_get_post failed!");
        return -1;
    }

    // for debug.
    /*
    log_txt_err("bs_rsp._list_num=%d", bs_rsp._list_num)
    for (int i=0; i<bs_rsp._list_num; i++)
    {
        int j=0 ;
        while (bs_rsp._lists[i][j] != 0)
        {
            log_txt_err("bs_rsp._lists[%d][%d]=%llu",i,j,bs_rsp._lists[i][j]) ;
            log_txt_err("bs_rsp._user_ids[%d][%d]=%llu",i,j,bs_rsp._user_ids[i][j]) ;
            log_txt_err("bs_rsp._user_types[%d][%d]=%llu",i,j,bs_rsp._user_types[i][j]) ;
            j++ ;
        }
    }
    log_txt_err("req->_start_idx=%d, req->_req_num=%d", req->_start_idx, req->_req_num) ;
    */
    
    // merge
    int in_list_x[MAX_RET_POST_NUM] ;
    int in_list_y[MAX_RET_POST_NUM] ;
    ret = multi_merge(bs_rsp._lists, bs_rsp._list_num, rsp->_posts, MAX_RET_POST_NUM,
                       in_list_x, in_list_y, req->_start_idx, req->_req_num);
    if (ret < 0) {
        log_txt_err("merge failed!") ;
        return -1;
    }
    for (int j=0; j<ret; j++)
    {
        rsp->_user_types[j] = bs_rsp._user_types[in_list_x[j]][in_list_y[j]] ;
        rsp->_user_ids[j] = bs_rsp._user_ids[in_list_x[j]][in_list_y[j]] ;
    }

    // for debug.
    /*
    log_txt_err("multi_merge num=%d", ret) ;
    for (int j=0; j<ret; j++)
    {
        log_txt_err("post id[%d]=%llu", j, rsp->_posts[j]) ;
        log_txt_err("user id[%d]=%llu", j, rsp->_user_ids[j]) ;
        log_txt_err("user type[%d]=%llu", j, rsp->_user_types[j]) ;
    }
    */

    rsp->_header.len = sizeof(as2is_get_post_t) - sizeof(rsp->_posts) + sizeof(rsp->_posts[0]) * ret;
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = 0;
    rsp->_post_num = ret;

    return 0;
}

//added by Radio
static int _proc_get_post_by_page(struct io_buff *buff)
{
    int ret = 0;
    is2as_get_post_t *req = (is2as_get_post_t *) buff->rbuff;
    as2is_get_post_t *rsp = (as2is_get_post_t *) buff->wbuff;

    bs_get_post_req_t bs_req;
    bs_req._user_num = req->_user_num;
    if ( bs_req._user_num > MAX_FOLLOWER_NUM-1 )
        bs_req._user_num = MAX_FOLLOWER_NUM - 1 ;

    // log_txt_err("uid:%llu\n", req->_user_ids[0]);

    bs_req._user_types = req->_user_types;
    bs_req._user_ids = req->_user_ids;
    bs_req._tag = req->_tag;
    
    /*
    bs_req._start_idx = req->_start_idx;
    bs_req._req_num = req->_req_num;
    */

    bs_req._start_idx = req->_start_idx;
    bs_req._req_num = bs_req._start_idx + req->_req_num ;


    bs_get_post_rsp_t bs_rsp;
    ret = bs_get_post_by_page(as_service._bsc, &bs_req, &bs_rsp, g_setting._get_post_max_time);
    if (ret < 0) {
        log_txt_err("bs_get_post failed!");
        return -1;
    }

    /*
    log_txt_err("bs_rsp._list_num=%d\n", bs_rsp._list_num)
    for (int i=0; i<bs_rsp._list_num; i++)
    {
        int j=0 ;
        while (bs_rsp._lists[i][j] != 0)
        {
            log_txt_err("bs_rsp._lists[%d][%d]=%llu",i,j,bs_rsp._lists[i][j]) ;
            log_txt_err("bs_rsp._user_ids[%d][%d]=%llu",i,j,bs_rsp._user_ids[i][j]) ;
            log_txt_err("bs_rsp._user_types[%d][%d]=%llu",i,j,bs_rsp._user_types[i][j]) ;
            //log_txt_err("bs_rsp._list_len[%d]", bs_rsp._list_len[i]) ;
            //log_txt_err("bs_rsp._list_num=%d", bs_rsp._list_num);
            j++ ;
        }
    }
    log_txt_err("req->_start_idx=%d, req->_req_num=%d", req->_start_idx, req->_req_num) ;
    */

    int actual_post_num = 0;
    int list_idx = bs_rsp._list_num - 1;
    int rsp_post_num = bs_rsp._list_len[list_idx];
    for(int j=0; j<rsp_post_num; j++) {
        rsp->_posts[actual_post_num] = bs_rsp._lists[list_idx][j];
        rsp->_user_types[actual_post_num] = bs_rsp._user_types[list_idx][j];
        rsp->_user_ids[actual_post_num] = bs_rsp._user_ids[list_idx][j];
        actual_post_num++;
    }
    
    rsp->_header.len = sizeof(as2is_get_post_t) - sizeof(rsp->_posts) + sizeof(rsp->_posts[0]) * actual_post_num;
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = 0;
    rsp->_post_num = actual_post_num;

    return 0;
}


static int _proc_post_set_scenic(struct io_buff *buff)
{
    is2as_post_set_scenic_t *req = (is2as_post_set_scenic_t *)buff->rbuff;
    as2is_post_set_scenic_t *rsp = (as2is_post_set_scenic_t *)buff->wbuff;
    
    bs_set_post_req_t bs_req;

    bs_req._user_ids[0]     = req -> _user_id;
    bs_req._user_num        = 1;
    bs_req._post_id         = req -> _post_id;
    bs_req._tag_num         = req -> _tag_num;
    for ( int i = 0; i < req->_tag_num; i++ )
        bs_req._tags[i] = req->_tags[i];

    if( 0 == req->_option )
    {
        if ( bs_set_post(as_service._bsc, &bs_req, SET_POST_USERID_SET, g_setting._get_post_max_time) < 0 )
        {
            log_txt_err("set post failed!");
            return -1;
        }
    }
    if( 1 == req->_option )
    {
        if ( bs_set_post(as_service._bsc, &bs_req, SET_POST_USERID_REMOVE, g_setting._get_post_max_time) < 0 )
        {
            log_txt_err("bs del post failure.");
            return -1;
        }
    }

    rsp->_header.len = sizeof(as2is_post_set_scenic_t);
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = 0;
    return 0;
}

static int _proc_del_post(struct io_buff *buff)
{
    int i;
    is2as_del_post_t *req = (is2as_del_post_t *) buff->rbuff;
    as2is_del_post_t *rsp = (as2is_del_post_t *) buff->wbuff;
    
    // 向 BS 发起请求.

    if (as_service._post2user == NULL && _init_post2user() < 0)
    {
        log_txt_err("post2user service init failure.") ;
        return -1;
    }
    
    bs_set_post_req_t bs_req;
    bs_req._post_id = req->_post_id;
    bs_req._tag_num  = req->_tag_num;
    for (i = 0; i < req->_tag_num; i++)
    {
        bs_req._tags[i] = req->_tags[i];
    }

    bs_req._user_num = _get_redis_list(as_service._post2user, req->_post_id, 
                                       0, MAX_REF_CNT_PER_POST + 1, PFX_POST2USER, 
                                       MAX_REF_CNT_PER_POST + 1, bs_req._user_ids);

    if ( bs_req._user_num < 0 )
    {
        log_txt_err("get post2user failed, post_id:[%llu]", (unsigned long long )req->_post_id);
        return -1;
    }
    if (bs_set_post(as_service._bsc, &bs_req, SET_POST_USERID_REMOVE, g_setting._get_post_max_time) < 0 )
    {
        log_txt_err("bs del post failure.");
        return -1;
    }
    
    // 删除 post2user 索引
    
    svr_group_t *rds = sm_get_svr(as_service._post2user, req->_post_id) ;
    if ( rds == NULL )
    {
        log_txt_err("get %s server failed, key:[%llu]", PFX_POST2USER, (unsigned long long) req->_post_id) ;
        return -1 ;
    }
    redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn, 
                         "DEL %s%llu", PFX_POST2USER, req->_post_id) ;
    if ( reply == NULL )
    {
        log_txt_err("execute redis command failed: [DEL %s%llu]", PFX_POST2USER, (unsigned long long) req->_post_id) ;
        sm_reconnect(as_service._post2user, rds) ;
        return -1 ;
    }
    freeReplyObject(reply) ;

    // 删除 post2at 索引
    
    if (as_service._post2at == NULL && _init_post2at() < 0)
    {
        log_txt_err("post2at service init failure.") ;
        return -1;
    }
    rds = sm_get_svr(as_service._post2at, req->_post_id) ;
    if ( rds == NULL )
    {
        log_txt_err("get %s server failed, key:[%llu]", PFX_POST2AT, (unsigned long long) req->_post_id) ;
        return -1 ;
    }
    reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn,
                         "DEL %s%llu", PFX_POST2AT, req->_post_id) ;
    if ( reply == NULL )
    {
        log_txt_err("execute redis command failed: [DEL %s%llu]", PFX_POST2AT, (unsigned long long) req->_post_id) ;
        sm_reconnect(as_service._post2at, rds) ;
        return -1 ;
    }
    freeReplyObject(reply) ;


    rsp->_header.len = sizeof(as2is_del_post_t);
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = 0;

    return 0;
}

static int extract_scenic_from_post(uint64_t post_id, const char* text,uint64_t user_ids[MAX_REF_CNT_PER_POST], int *user_num)
{
    // 默认一篇文章最大含有1000个旅行地（含重复）. 如果超出，则认为文章异常.

    dm_pack_t *ppack = dm_pack_create(1000);
    if (ppack == NULL) {
        log_txt_err("dm_pack_create fail, cnt:[%d]", MAX_REF_CNT_PER_POST);
        return -1;
    }
    
    /*   1. 对文本内容(标题和正文)进行旅行地库的多模匹配.                      */
    /*                                                                         */
    /*   旅行地和用户共享一套ID体系. 对一 post 而言, 除了作者以外, 它还可以和  */
    /*   提及的旅行地关联在一起. 在某个旅行地页下查看和该景点关联的post, 原理  */
    /*   和在个人页看该用户的所有 post 一样.                                   */
    /*                                                                         */
    /*   bs_req_user_ids[] 中, 0号元素为 req->_user_id, 后接1个或多个旅行地.   */

    int len = strlen(text);
    
    int ret= dm_search(g_setting._key_word_dict, ppack, text, len, DM_OUT_FMM,DM_CHARSET_UTF8) ;
    if ( ret < 0) 
    {
        log_txt_err("dm_search failed, text:[%s]", text);
    }

    // 文章异常, 包含过多旅行地
    
    else if (ret == 2)
    {
        log_txt_err("too many scenics found by dm_search, pid:%llu", post_id);
    }
    else
    {
        /*   匹配旅行地时, 记得排重.   */
        
        int num = *user_num;
        dm_lemma_t **elements = ppack->ppseg;
        for (int i = 0; i < (int)(ppack->ppseg_cnt) && num < MAX_REF_CNT_PER_POST; i++)
        {
            int j = 0 ;
            while (j<num)
            {
                if (elements[i]->prop == user_ids[j])
                {
                    break ;
                }
                j++;
            }
            if ( j == num )
            {
                user_ids[num] = elements[i]->prop;
                num++;
            }
        }
        *user_num = num;
    }
    dm_pack_del(ppack);
    return 0;
}

/*      发表内容时, 对外接口不区分是新发表还是修改原来的内容. 实际执行业务逻辑时  */
/*  判断 _post_id 是否出现在索引中就可以判断出来. 如果是修改类, 对于提及景点,     */
/*  删除不再提及景点的索引项, 增加新的索引项; 对于 at 用户, 不删除不再被 at 的用  */
/*  户的索引项，直接追加提醒新被 at 的用户.                                       */

static int _proc_set_post(struct io_buff *buff)
{
    is2as_set_post_t *req = (is2as_set_post_t *) buff->rbuff;
    as2is_set_post_t *rsp = (as2is_set_post_t *) buff->wbuff;
    
    bs_set_post_req_t bs_req;

    bs_req._user_ids[0]     = req -> _user_id;
    bs_req._user_num        = 1;
    bs_req._post_id         = req -> _post_id;
    bs_req._tag_num         = req -> _tag_num;
    for ( int i = 0; i < req->_tag_num; i++ )
        bs_req._tags[i] = req->_tags[i];

    if (as_service._post2user == NULL && _init_post2user() < 0)
    {
        log_txt_err("post2user service init failure.") ;
        return -1;
    }

    /*
     *     1. 对内容中提到的旅行地进行识别. 
     *
     *     如果是转发生成的文章, 那么去拉取原文的旅行地列表; 本文的旅行地列表, 必须
     * 包含在原文的旅行地列表之中; 不在的则排除掉.
     *
     */
    if (req->_extract_scenic_option)
    {
        uint64_t post_scenic_list[MAX_REF_CNT_PER_POST] ;
        int      post_scenic_list_num = 0 ;

        extract_scenic_from_post( req->_post_id, req->_ref_text, post_scenic_list, &post_scenic_list_num );
        
        if ( post_scenic_list_num > 0 )
        {
            int ref_post_scenic_num = 0 ;
            uint64_t ref_post_scenic_list[MAX_REF_CNT_PER_POST+1];

            if( 0 != req -> _ref_pid )
            {
                ref_post_scenic_num = _get_redis_list(as_service._post2user, req->_ref_pid, 
                                                      0, MAX_REF_CNT_PER_POST + 1, PFX_POST2USER,
                                                      MAX_REF_CNT_PER_POST + 1, ref_post_scenic_list );
            }

            for (int i=0; i<post_scenic_list_num; i++)
            {
                /* 原文 */
                if ( ref_post_scenic_num == 0 )
                {
                    bs_req._user_ids[bs_req._user_num] = post_scenic_list[i] ;
                    bs_req._user_num ++ ;
                }
                /* 转发文; 且原文没有旅行地. */
                else if ( ref_post_scenic_num == 1 )
                {
                    break ;
                }
                /* 转发文; 且原文有旅行地. */
                else
                {
                    for (int j=0; j<ref_post_scenic_num; j++)
                    {
                        if (post_scenic_list[i] == ref_post_scenic_list[j])
                        {
                            bs_req._user_ids[bs_req._user_num] = post_scenic_list[i] ;
                            bs_req._user_num ++ ;
                            break ;
                        }
                    }
                }
            }
        }
    }

    /*   2. 判断是第一次发表的新内容, 还是修改原来的内容. 如果是后者, 则抽取 */
    /*   出新提及的景点, 和被取消提及的景点.                                 */

    bool isNewPost = true ;

    int new_user_cnt = 0 ;
    uint64_t new_user_id_list[MAX_REF_CNT_PER_POST+1] ;

    int del_user_cnt = 0 ;
    uint64_t del_user_id_list[MAX_REF_CNT_PER_POST+1] ;

    uint64_t old_user_id_list[MAX_REF_CNT_PER_POST+1] ;
    int old_user_cnt = _get_redis_list(as_service._post2user, req->_post_id, 
                                       0, MAX_REF_CNT_PER_POST + 1, PFX_POST2USER, 
                                       MAX_REF_CNT_PER_POST + 1, old_user_id_list);
    if ( old_user_cnt < 0 ) {
        log_txt_err("get post2user failed, post_id:[%llu]", (unsigned long long)req->_post_id);
        // return -1;
    }
    /* 比较, 寻找差异 */
    else if ( old_user_cnt > 0 )
    {
        isNewPost = false ;
        for (int i=0; i<bs_req._user_num; i++)
        {
            int j = 0 ;
            while ( j < old_user_cnt )
            {
                if (bs_req._user_ids[i] == old_user_id_list[j])
                {
                    break ;
                }
                j++ ;
            }
            if (j == old_user_cnt)
            {
                new_user_id_list[new_user_cnt] = bs_req._user_ids[i] ;
                new_user_cnt ++ ;
            }
        }
        for (int j=0; j<old_user_cnt; j++) 
        {
            int i = 0 ;
            while ( i < bs_req._user_num ) 
            {
                if (bs_req._user_ids[i] == old_user_id_list[j]) 
                {
                    break ;
                }
                i ++ ;
            }
            if ( i == bs_req._user_num ) 
            {
                del_user_id_list[del_user_cnt] = old_user_id_list[j] ;
                del_user_cnt ++ ;
            }
        }
    }
    if ( isNewPost == false) 
    {
        if ( new_user_cnt > 0 ) 
        {
            for (int i=0; i<new_user_cnt; i++) 
            {
                bs_req._user_ids[i] = new_user_id_list[i] ;
            }
            bs_req._user_num = new_user_cnt ;
        }
        else
        {
            bs_req._user_num = 0 ;
        }
    }
    /*   3. 不管是新发表还是修改内容, 只要涉及新提及的景点, 就更新 post2user   */
    /*   索引, 调用BS模块.                                                     */
    
    /*    2016-2-17 更改: 如果是修改内容时景点增加了, 不修改对应post2user索引. */

    //if (bs_req._user_num > 0)
    if (isNewPost && bs_req._user_num > 0)
    {
        for (int j=0; j<bs_req._user_num; j++)
        {
            int ret = _set_redis_list(as_service._post2user, PFX_POST2USER, 
                            req->_post_id, bs_req._user_ids[j]);
            if (ret < 0)
            {
                log_txt_err("set post2user failed, post_id:[%llu] user_id:[%llu]", 
                            (unsigned long long)req->_post_id, (unsigned long long)bs_req._user_ids[j]);
                // return -1;
            }
        }
        
        int ret = bs_set_post(as_service._bsc, &bs_req, SET_POST_USERID_SET,g_setting._get_post_max_time);
        if (ret < 0)
        {
            log_txt_err("set post failed!");
            return -1;
        }
    }
    
    /*   4.1. 如果是修改内容且部分景点被取消提及, 删除对应 post2user 索引, 并   */
    /*   调用BS模块进行脱钩.                                                    */

    /*   2016-2-17 修改: 如果是修改内容时部分景点取消, 不删除对应post2user索引. */

    /*
     if ( isNewPost == false && del_user_cnt > 0)
     {
         for (int j=0; j<del_user_cnt; j++)
         {
             int ret = _remove_redis_list(as_service._post2user, PFX_POST2USER, 
                             req->_post_id, del_user_id_list[j]);
             if (ret < 0) {
                 log_txt_err("remove post2user post failed, post_id:[%llu] user_id:[%llu]", 
                             (unsigned long long)req->_post_id, (unsigned long long)del_user_id_list[j]);
                 // return -1;
             }

             bs_req._user_ids[j] = del_user_id_list[j] ;
         }
         bs_req._user_num = del_user_cnt ;
         int ret = bs_set_post(as_service._bsc, &bs_req, SET_POST_USERID_REMOVE,g_setting._get_post_max_time);
         if (ret < 0)
         {
             log_txt_err("set post failed!");
             return -1;
         }
     }
     */

    /*   4.2. 如果是新发表内容, 推送至粉丝首页.                                */

    if ( isNewPost )
    {
        if (as_service._push == NULL && _init_push() < 0)
        {
            log_txt_err("push service init failure. push service aborted.") ;
        }
        else if (_set_push(as_service._push, PUSH_SERVICE_TYPE_TIMELINE, req->_user_id,req->_post_id) < 0) 
        {
            log_txt_err("push failed! user_id:[%llu], post_id:[%llu]", (unsigned long long)req->_user_id, 
                        (unsigned long long)req->_post_id);
        }
    }

    /*   5. 当 post 中 at 其他用户后, 修改 atme 数据索引, 并通知被at者. 注意,  */
    /*   如果是修改文章, 且更改了 at 的用户, 那么针对被取消at的用户, 不做更改, */
    /*   对新被 at 的用户, 追加提醒.                                           */

    if (as_service._post2at == NULL && _init_post2at() < 0)
    {
        log_txt_err("post2at service init failure.") ;
        return -1;
    }
    if (req->_at_user_num != 0 && !isNewPost)
    {
        uint64_t old_at_user_list[MAX_AT_USER_NUM+1] ;
        int old_at_user_cnt = _get_redis_list(as_service._post2at, req->_post_id, 
                                   0, MAX_AT_USER_NUM+1, PFX_POST2AT, 
                                   MAX_AT_USER_NUM+1, old_at_user_list);
        if ( old_at_user_cnt < 0 )
        {
            log_txt_err("get post2at failed, post_id:[%llu]", (unsigned long long)req->_post_id);
            return -1;
        }
        
        /* 比较, 寻找差异 */

        else if ( old_at_user_cnt > 0 )
        {
            int new_at_cnt = 0 ;
            for (int i=0; i<req->_at_user_num; i++)
            {
                int j = 0;
                while( j < old_at_user_cnt ) 
                {
                    if (req->_at_users[i] == old_at_user_list[j])
                        break ;
                    j++ ;
                }
                if ( j == old_at_user_cnt)
                {
                    req->_at_users[new_at_cnt] = req->_at_users[i] ;
                    new_at_cnt ++ ;
                }
            }
            req->_at_user_num = new_at_cnt ;
        }
    }

    /* 对比之后发现确实有新 at 用户, 则提醒并修改 post2at 索引.  */

    if ( req->_at_user_num != 0 && req->_at_user_num < MAX_AT_USER_NUM)
    {
        _at_users(req->_user_id, req->_post_id, req->_at_user_num, req->_at_users);

        for (int i=0; i<req->_at_user_num; i++)
        {
            int ret = _set_redis_list(as_service._post2at, PFX_POST2AT, 
                                req->_post_id, req->_at_users[i]);
            if (ret < 0)
            {
                log_txt_err("set post2at failed, post_id:[%llu] user_id:[%llu]", 
                            (unsigned long long)req->_post_id, (unsigned long long)req->_at_users[i]);
            }
        }
    }

    // pack response
    rsp->_header.len = sizeof(as2is_set_post_t);
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = 0;

    return 0;
}

static int _proc_get_post_cnt(struct io_buff *buff)
{
    int post_cnt = 0;
    is2as_get_post_cnt_t *req = (is2as_get_post_cnt_t *)buff->rbuff;
    as2is_get_post_cnt_t *rsp = (as2is_get_post_cnt_t *)buff->wbuff;

    bs_get_post_cnt_req_t bs_req;
    bs_req._user_num = req->_user_num;
    bs_req._user_ids = req->_user_ids;
    bs_req._tag = req->_tag;

    post_cnt = bs_get_post_cnt(as_service._bsc, &bs_req, g_setting._get_post_max_time);
    if (post_cnt < 0) {
        log_txt_err("bs_get_post failed!");
        return -1;
    }
    
    // pack response
    rsp->_header.len = sizeof(as2is_get_post_cnt_t);
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = 0;
    rsp->_post_num = post_cnt;

    return 0;
}

static int _proc_get_post_atme(struct io_buff *buff)
{
    if (as_service._atme == NULL && _init_atme() < 0) {
        return -1;
    }

    is2as_get_post_atme_t *req = (is2as_get_post_atme_t *) buff->rbuff;
    as2is_get_post_atme_t *rsp = (as2is_get_post_atme_t *) buff->wbuff;

    int post_cnt = _get_redis_list(as_service._atme, req->_user_id, 
                                   req->_start_idx, req->_req_num,
                                   PFX_ATME, MAX_RET_POST_NUM, rsp->_posts);

    if (post_cnt < 0) {
        log_txt_err("get post atme failed, user_id:[%llu]", (unsigned long long)req->_user_id);
        return -1;
    }

    rsp->_header.len = sizeof(as2is_get_post_atme_t) - sizeof(rsp->_posts) + post_cnt * sizeof(rsp->_posts[0]) ;
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = 0;
    rsp->_post_num = post_cnt;

    return 0;
}

static int _proc_get_post_atme_cnt(struct io_buff *buff)
{
    if (as_service._atme == NULL && _init_atme() < 0) {
        return -1;
    }
    return _get_as_item_cnt(buff, as_service._atme, PFX_ATME);
}

static int _proc_set_enshrine(struct io_buff *buff)
{
    if (as_service._enshrine == NULL && _init_enshrine() < 0) {
        return -1;
    }

    is2as_set_enshrine_t *req = (is2as_set_enshrine_t *) buff->rbuff;
    as2is_set_enshrine_t *rsp = (as2is_set_enshrine_t *) buff->wbuff;

    std::string value ;
    if ( req->_obj_type < 0 )
    {
        log_txt_err("set/unset argument 2 invalid: %d", req->_obj_type) ;
        return -1 ;
    }

    /* 当对象类型不为0(post)时, 要将类型和ID以组合字符串形式写入enshrine的sorted set. */

    else if ( req->_obj_type > 0 )
    {
        value = StringUtil::Uint64ToStr(req->_obj_type)+";"+StringUtil::Uint64ToStr(req->_obj_id) ;
    }

    int ret  = 0 ;
    if ( req->set_or_unset == (int) SET_ENSHRINE_TYPE_SET )
    {
        if ( req->_obj_type == 0 )
            ret = _set_redis_zset(as_service._enshrine, PFX_ENSHRINE, req->_user_id, req->_obj_id);
        else
            ret = _set_redis_zset_string(as_service._enshrine, PFX_ENSHRINE, req->_user_id, value);
    }
    else if ( req->set_or_unset == (int) SET_ENSHRINE_TYPE_UNSET )
    {
        if ( req->_obj_type == 0 )
            ret = _remove_redis_zset(as_service._enshrine, PFX_ENSHRINE, req->_user_id, req->_obj_id) ;
        else
            ret = _remove_redis_zset_string(as_service._enshrine, PFX_ENSHRINE, req->_user_id, value) ;
    }
    else
    {
        log_txt_err("set/unset argument 4 invalid: %d", req->set_or_unset) ;
        return -1 ;
    }
    if (ret < 0)
    {
        log_txt_err("set/unset enshrine obj failed, user_id:[%llu] obj_id:[%llu]", 
                    (unsigned long long)req->_user_id, (unsigned long long)req->_obj_id);
        return -1;
    }

    rsp->_header.len = sizeof(as2is_set_enshrine_t);
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = 0;

    return 0;
}

static int _proc_get_enshrine(struct io_buff *buff)
{
    if (as_service._enshrine == NULL && _init_enshrine() < 0) {
        return -1;
    }

    is2as_get_enshrine_t *req = (is2as_get_enshrine_t *) buff->rbuff;
    as2is_get_enshrine_t *rsp = (as2is_get_enshrine_t *) buff->wbuff;

    int obj_cnt = _get_redis_zset_hybrid(as_service._enshrine, req->_user_id, 
                                         req->_start_idx, req->_req_num,
                                         PFX_ENSHRINE, MAX_RET_POST_NUM, rsp->_obj_types, rsp->_obj_ids);
    if (obj_cnt < 0) {
        log_txt_err("get enshrine failed, user_id:[%llu]", (unsigned long long)req->_user_id);
        log_txt_err("as_service._enshrine sharding type:%d", as_service._enshrine->_sharding_type) ;
        return -1;
    }

    /*
     * 收藏功能, 一开始只支持post类型, 后来支持问题, 答案, 各种产品等.
     *
     * 当 req->_req_type == 0 时代表这是一开始App版本发出来的请求, 当时只支持收藏 post 内容.
     * 因此当获取到所有收藏内容后, 过滤出 post 内容及相关数据后, 返回.
     *
     * 当 req->_req_type == 1 时代表请求所有类型的收藏内容, 包括 post, 问题, 答案, 行程, 等.
     *
     */

    if ( req->_req_type==0 && obj_cnt>0 )
    {
        int hybrid_cnt = obj_cnt ;
        int awaiting_idx = 0 ;
        for (int i=0; i<hybrid_cnt; i++)
        {
            if ( rsp->_obj_types[i] != 0 )
            {
                obj_cnt -- ;
                continue ;
            }
            rsp->_obj_ids[awaiting_idx] = rsp->_obj_ids[i] ;
            rsp->_obj_types[awaiting_idx] = rsp->_obj_types[i] ;

            awaiting_idx ++ ;
        }
    }
    rsp->_header.len = sizeof(as2is_get_enshrine_t) - sizeof(rsp->_obj_ids) + obj_cnt * sizeof(rsp->_obj_ids[0]) ;
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = 0;
    rsp->_obj_num = obj_cnt;
    
    return 0;
}

static int _proc_get_enshrine_cnt(struct io_buff *buff)
{
    if (as_service._enshrine == NULL && _init_enshrine() < 0) {
        return -1;
    }
    return _get_as_item_cnt(buff, as_service._enshrine, PFX_ENSHRINE);
}

static int _proc_set_paiduser(struct io_buff *buff)
{
    if (as_service._post2paiduser == NULL && _init_post2paiduser() < 0)
    {
        return -1;
    }

    is2as_set_paiduser_t *req = (is2as_set_paiduser_t *)buff->rbuff;
    as2is_set_paiduser_t *rsp = (as2is_set_paiduser_t *)buff->wbuff;

    if (req->_user_id == 0)
    {
        log_txt_err("is2as_set_paiduser: _user_id equals 0.");
        return -1;
    }
    if (req->_post_id == 0)
    {
        log_txt_err("is2as_set_paiduser: _post_id equals 0.");
        return -1;
    }
    int ret = 0;
    ret = _set_redis_list(as_service._post2paiduser, PFX_POST2PAIDUSER, 
                              req->_post_id, req->_user_id);
    if (ret < 0)
    {
        log_txt_err("set post2paiduser failed, post_id:[%llu] user_id:[%llu]", 
                    (unsigned long long)req->_post_id, (unsigned long long)req->_user_id);
        return -1;
    }
    
    rsp->_header.len = sizeof(as2is_set_paiduser_t);
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = 0;
    return 0;
}

static int _proc_get_paiduser(struct io_buff *buff)
{
    if (as_service._post2paiduser == NULL && _init_post2paiduser() < 0)
    {
        return -1;
    }

    is2as_get_paiduser_t *req = (is2as_get_paiduser_t *)buff->rbuff;
    as2is_get_paiduser_t *rsp = (as2is_get_paiduser_t *)buff->wbuff;

    int user_num = _get_redis_list(as_service._post2paiduser, req->_post_id, 
                                   req->_start_idx, req->_req_num,
                                   PFX_POST2PAIDUSER, MAX_PAIDUSER_CNT_PER_POST, rsp->_user_ids);

    if (user_num < 0) {
        log_txt_err("get post2paiduser failed, post_id:[%llu]", (unsigned long long)req->_post_id);
        return -1;
    }

    rsp->_header.len = sizeof(as2is_get_paiduser_t) - sizeof(rsp->_user_ids) + user_num * sizeof(rsp->_user_ids[0]) ;
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = 0;
    rsp->_user_num = user_num;

    return 0;
}

static int _proc_get_post2user(struct io_buff *buff)
{
    if (as_service._post2user == NULL && _init_post2user() < 0)
    {
        return -1;
    }

    is2as_get_scenic_user_by_post_t *req = (is2as_get_scenic_user_by_post_t *)buff->rbuff;
    as2is_get_scenic_user_by_post_t *rsp = (as2is_get_scenic_user_by_post_t *)buff->wbuff;

    int scenic_user_num = _get_redis_list(as_service._post2user, req->_post_id, 
                                   req->_start_idx, req->_req_num,
                                   PFX_POST2USER, MAX_REF_CNT_PER_POST, rsp->_scenic_user_ids);

    if (scenic_user_num < 0) {
        log_txt_err("get post2user failed, post_id:[%llu]", (unsigned long long)req->_post_id);
        return -1;
    }

    rsp->_header.len = sizeof(as2is_get_scenic_user_by_post_t) - sizeof(rsp->_scenic_user_ids) + scenic_user_num * sizeof(rsp->_scenic_user_ids[0]) ;
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = 0;
    rsp->_scenic_user_num = scenic_user_num;

    return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

int as_init(const char *conf_file)
{
    int ret = 0;

    ret = _read_conf(conf_file);
    if (ret < 0) {
        return -1;
    }

    return 0;
}

int as_proc(struct io_buff *buff)
{
    if (as_service._bsc == NULL) {
        as_service._bsc = bs_init(g_setting._bs_info, g_setting._bs_group_num);
        if (as_service._bsc == NULL) {
            log_txt_err("initialize bs failed!");
            return -1;
        }
    }

    req_pack_t *req = (req_pack_t *) buff->rbuff;

    int ret = 0;
    switch (req->_header.cmd) {
        case CMD_GET_POST:
            ret = _proc_get_post(buff);
            break;
        
        case CMD_GET_POST_BY_PAGE:
            ret = _proc_get_post_by_page(buff);
            break;

        case CMD_SET_POST:
            ret = _proc_set_post(buff);
            break;
        
        case CMD_DEL_POST:
            ret = _proc_del_post(buff);
            break;

        case CMD_GET_POST_CNT:
            ret = _proc_get_post_cnt(buff);
            break;

        case CMD_GET_POST_ATME:
            ret = _proc_get_post_atme(buff);
            break;

        case CMD_SET_ENSHRINE:
            ret = _proc_set_enshrine(buff);
            break;

        case CMD_GET_ENSHRINE:
            ret = _proc_get_enshrine(buff);
            break;
        
        case CMD_GET_ENSHRINE_CNT:
            ret = _proc_get_enshrine_cnt(buff);
            break;
        
        case CMD_GET_POST_ATME_CNT:
            ret = _proc_get_post_atme_cnt(buff);
            break;

        case CMD_SET_PAIDUSER:
            ret = _proc_set_paiduser(buff);
            break;

        case CMD_GET_PAIDUSER:
            ret = _proc_get_paiduser(buff);
            break;
        
		case CMD_GET_SCENIC_USER_BY_POST:
            ret = _proc_get_post2user(buff);
            break;

        case CMD_POST_SET_SCENIC:
            ret = _proc_post_set_scenic(buff);
            break;

        default:
            log_txt_err("unknown command number:[%d]", req->_header.cmd);
            ret = -1;
            break;
    }

    if (ret < 0) {
        _build_failed_pack(buff);
    }

    return ret;
}

int as_uninit()
{
    if ( g_setting._key_word_dict != NULL ) {
        dm_dict_del(g_setting._key_word_dict) ;
        g_setting._key_word_dict = NULL ;
    }
    if (as_service._bsc) {
        bs_free(as_service._bsc);
        as_service._bsc = NULL;
    }
    if (as_service._atme) {
        sm_uninit(as_service._atme);
        as_service._atme = NULL;
    }
    if (as_service._enshrine) {
        sm_uninit(as_service._enshrine);
        as_service._enshrine = NULL;
    }
    if (as_service._post2user) {
        sm_uninit(as_service._post2user);
        as_service._post2user = NULL;
    }
    if (as_service._post2at) {
        sm_uninit(as_service._post2at);
        as_service._post2at = NULL;
    }
    if (as_service._post2paiduser) {
        sm_uninit(as_service._post2paiduser);
        as_service._post2paiduser = NULL;
    }
    if (as_service._push) {
        sm_uninit(as_service._push);
        as_service._push = NULL;
    }
    return 0 ;
}

#ifdef __cplusplus
}
#endif
