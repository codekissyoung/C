#include "hiredis.h"
#include "server_manager.h"
#include "common.h"
#include "interface.h"
#include "log.h"
#include "conf.h"
#include "buffer.h"

typedef struct {
    char _post2comment_conf_file[MAX_FILE_PATH_LEN];
    char _comment2post_conf_file[MAX_FILE_PATH_LEN];
    char _comment2comment_conf_file[MAX_FILE_PATH_LEN];
    char _user2comment_conf_file[MAX_FILE_PATH_LEN];
	/*
    char _user_comment_conf_file[MAX_FILE_PATH_LEN];
	*/
    char _atme_cmt_conf_file[MAX_FILE_PATH_LEN];

    char _push_conf_file[MAX_FILE_PATH_LEN];

    char _product2comment_conf_file[MAX_FILE_PATH_LEN];
    char _productcmt2comment_conf_file[MAX_FILE_PATH_LEN];
    char _user2productcmt_conf_file[MAX_FILE_PATH_LEN];

} cs_conf_t;

typedef struct {
    svr_mgr_t *_post2comment; 
    svr_mgr_t *_comment2post;
    svr_mgr_t *_comment2comment;
    svr_mgr_t *_user2comment; //用户收到的评论
/*
	svr_mgr_t *_user_comment;
	*/
    svr_mgr_t *_atme_cmt;
    svr_mgr_t *_push;
    
    svr_mgr_t *_product2comment;
    svr_mgr_t *_productcmt2comment;
    svr_mgr_t *_user2productcmt;
} comment_service_t;

const int redis_cmd_len = 1024 * 200;
cs_conf_t g_setting;
//__thread comment_service_t comment_service = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
__thread comment_service_t comment_service = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

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
    if (*svr != NULL)
    {
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
static int _set_redis_list(svr_mgr_t *svr_handler, const char *pfx, uint64_t key, uint64_t value)
{
    if (svr_handler == NULL)
    {
        return -1;
    }

    svr_group_t *rds = sm_get_svr(svr_handler, key);
    if (rds == NULL) {
        log_txt_err("get %s server failed, key:[%llu]", pfx, (unsigned long long)key);
        return -1;
    }

    redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn,
            "LPUSH %s%llu %llu", pfx, key, value);
    if (reply == NULL)
    {
        log_txt_err("execute redis command failed: [LPUSH %s%llu %llu]", pfx, 
                (unsigned long long)key, (unsigned long long)value);
        sm_reconnect(svr_handler, rds);
        return -1;
    }

    freeReplyObject(reply);

    return 0;
}

static int _redis_lrem(svr_mgr_t *svr_handler, const char *pfx,
        uint64_t key, uint64_t value)
{
    if (svr_handler == NULL)
        return -1;

    svr_group_t *rds = sm_get_svr(svr_handler, key);
    if (rds == NULL)
    {
        log_txt_err("get %s server failed, key:[%llu]", pfx, (unsigned long long)key);
        return -1;
    }

    redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn,
            "LREM %s%llu %d %llu", pfx, key, 0, value);
    if (reply == NULL)
    {
        log_txt_err("execute redis command failed: [LREM %s%llu %d %llu]",
                pfx, (unsigned long long)key, 0, value);
        sm_reconnect(svr_handler, rds);
        return -1;
    }
    int ret = 0;
    if (reply->type == REDIS_REPLY_INTEGER)
    {
        ret = reply->integer;
    }
    freeReplyObject(reply);
    return ret;
}

static int _redis_key_del(svr_mgr_t *svr_handler, const char *pfx, uint64_t key)
{
    if (svr_handler == NULL) return -1;
    svr_group_t *rds = sm_get_svr(svr_handler, key);
    if (rds == NULL) {
        log_txt_err("get %s server failed, key:[%llu]", pfx, (unsigned long long)key);
        return -1;
    }

    redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn,
            "DEL %s%llu", pfx, key);
    if (reply == NULL)
    {
        log_txt_err("execute redis command failed: [DEL %s%llu]",
                pfx, (unsigned long long)key);
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
    if (svr_handler == NULL) {
        return -1;
    }

    svr_group_t *rds = sm_get_svr(svr_handler, key);
    if (rds == NULL) {
        log_txt_err("get %s server failed, key:[%llu]", pfx, (unsigned long long)key);
        return -1;
    }

    redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn, "LRANGE %s%llu %d %d", 
            pfx, key, start_idx, start_idx + req_num-1);
    if (reply == NULL)
    {
        log_txt_err("execute redis command failed: [LRANGE %s%llu %d %d]", 
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

static int _get_comment(struct io_buff *buff, svr_mgr_t *svr_handler, const char *pfx)
{
    if (svr_handler == NULL) {
        return -1;
    }

    is2cs_get_comment_t *req = (is2cs_get_comment_t *)buff->rbuff;
    cs2is_get_comment_t *rsp = (cs2is_get_comment_t *)buff->wbuff;

    int elementSize = _get_redis_list(svr_handler, req->_obj_id, req->_start_idx, req->_req_num,
            pfx, MAX_RET_COMMENT_NUM, rsp->_comments) ;
    if ( elementSize < 0 )
        return -1 ;

    rsp->_comment_num = elementSize ;
    rsp->_header.len = sizeof(cs2is_get_comment_t)-sizeof(rsp->_comments)+rsp->_comment_num*sizeof(rsp->_comments[0]);
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = 0;


    return 0;
}

/* CS 模块中只需要负责2种类型消息的推送服务: 评论我的, at我的评论. */

static int _set_push(svr_mgr_t *svr_handler, int push_service_type, uint64_t user_id, uint64_t action_uid) 
{
    if ( push_service_type != PUSH_SERVICE_TYPE_ATME_CMT && 
         push_service_type != PUSH_SERVICE_TYPE_NEW_PRODUCT_COMMENT &&
         push_service_type != PUSH_SERVICE_TYPE_NEWCOMMENT) {

        log_txt_err("push_service_type not valid: %d", push_service_type) ;
        return -1 ;
    }

    /* 获取 push 句柄 */
    svr_group_t *rds = sm_get_svr(svr_handler, user_id);
    if (rds == NULL) {
        log_txt_err("get %s server failed, user_id:[%llu]", PFX_PUSH, (unsigned long long)user_id);
        return -1;
    }

    char cmd[redis_cmd_len];
    memset(cmd, 0, sizeof(cmd)) ;
    snprintf(cmd, sizeof(cmd), "LPUSH %s {\"type\":%d,\"content\":{\"uid\":\"%llu\",\"action_uid\":\"%llu\"}}", 
            PFX_PUSH, push_service_type, (unsigned long long)user_id,(unsigned long long)action_uid);

    /* 写入 push 消息队列 */
    redisReply *reply = (redisReply*)redisCommand((redisContext*)rds->_cur_conn, cmd);
    if (reply == NULL) {
        log_txt_err("execute redis command faild, command: %s", cmd);
        sm_reconnect(comment_service._push, rds);
        return -1;
    }

    freeReplyObject(reply);

    return 0;
}

static int _set_reply(uint64_t post_id, uint64_t parent_comment_id, uint64_t comment_id)
{
    /*    1. 获取 parent_comment_id 的 post_id list, 然后将comment_id 增加进这些 */
    /*  post_id 的 post2comment 索引中.                                          */

    uint64_t parent_cmt_posts[MAX_RET_POST_NUM] ;
    int elementSize = _get_redis_list(comment_service._comment2post, parent_comment_id, 0, 0,
            PFX_COMMENT2POST, MAX_RET_COMMENT_NUM, parent_cmt_posts) ;
    if ( elementSize < 0 )
        return -1 ;
    for ( int i=0; i<elementSize; i++)
    {
        if (parent_cmt_posts[i] == post_id)
            continue;
        _set_redis_list(comment_service._post2comment, PFX_POST2COMMENT, parent_cmt_posts[i], comment_id);
        _set_redis_list(comment_service._comment2post, PFX_COMMENT2POST, comment_id, parent_cmt_posts[i]);
    }

    /*    2. 获取 parent_comment_id 的溯源 comment_id list. 针对当前 comment_id  */
    /*  为key 新建一个 comment2comment 索引, 将 parent_comment_id 和溯源出来的   */
    /*  comment_id list 增加进当前 comment_id 的 comment2comment 索引中.         */

    /*  注: 可参考雪球网的"查看对话" 功能.                                       */

    /*   2.1. */
    uint64_t tmp_comment_ids[MAX_CMT_NUM_PER_THREAD-1] ;
    elementSize = _get_redis_list(comment_service._comment2comment, parent_comment_id, 0, 0,
            PFX_COMMENT2COMMENT, MAX_CMT_NUM_PER_THREAD-1, tmp_comment_ids) ;
    if ( elementSize < 0 )
        return -1 ;

    /*   2.2. */
    char cmd[redis_cmd_len];
    char *ptr = cmd;
    int lft_len = sizeof(cmd);
    int ret = snprintf(ptr, lft_len, "RPUSH %s%llu %llu", PFX_COMMENT2COMMENT, 
            (unsigned long long)comment_id, (unsigned long long)parent_comment_id);
    lft_len -= ret;
    ptr += ret;

    for (int i=0; i<elementSize; i++)
    {
        ret = snprintf(ptr, lft_len, " %llu", (unsigned long long)tmp_comment_ids[i]);
        lft_len -= ret;
        ptr += ret;
        if (lft_len < 0)
        {
            break;
        }
    }
    if (lft_len < 0)
    {
        log_txt_err("build cmd failed, cmd-buff was full, buff-size:[%d],"
                "LPUSH element size:[%d]", (int)sizeof(cmd), elementSize);
        return -1;
    }

    svr_group_t *rds = sm_get_svr(comment_service._comment2comment, comment_id);
    if (rds == NULL)
    {
        log_txt_err("get %s server failed, comment_id:[%lluu]", 
                PFX_COMMENT2COMMENT, (unsigned long long)comment_id);
        return -1;
    }

    redisReply *reply = (redisReply*)redisCommand((redisContext*)rds->_cur_conn, cmd);
    if (reply == NULL)
    {
        log_txt_err("execute redis command failed, command:[%s]", cmd);
        sm_reconnect(comment_service._comment2comment, rds);
        return -1;
    }
    freeReplyObject(reply);

    return 0;
}

static int _at_users(uint64_t user_id, uint64_t comment_id, int at_user_num, uint64_t *users)
{
#ifdef _DEBUG
    log_txt_info("at_user_num:%d", at_user_num);
#endif
    for (int i = 0; i < at_user_num; i++) {

#ifdef _DEBUG
        log_txt_info("at_user, comment_id:%" PRIu64 ", user_id:%" PRIu64" ", comment_id, users[i]);
#endif
        if (_set_redis_list(comment_service._atme_cmt, PFX_ATME_CMT, users[i], comment_id) < 0)
        {
            log_txt_err("at user failed! user_id:[%llu], comment_id:[%llu]", (unsigned long long)users[i], 
                    (unsigned long long)comment_id);
        }

        if (_set_push(comment_service._push, PUSH_SERVICE_TYPE_ATME_CMT, users[i], user_id) < 0)
        {
            log_txt_err("push failed! user_id:[%llu], comment_id:[%llu]", (unsigned long long)users[i], 
                    (unsigned long long)comment_id);
        }
    }
    return 0;
}

static int _set_comment(struct io_buff *buff)
{
    is2cs_set_comment_t *req = (is2cs_set_comment_t *)buff->rbuff;
    cs2is_set_comment_t *rsp = (cs2is_set_comment_t *)buff->wbuff;
    int ret = 0;

    if (req->_post_id == 0 && req->_parent_comment_id == 0) {
        log_txt_err("is2cs_set_comment _post_id and _parent_comment_id both 0!");
        return -1;
    }

	/*
    if (_set_redis_list(comment_service._user_comment, PFX_USER_COMMENT,
                req->_user_id, req->_comment_id) < 0) {
		log_txt_err("set user_comment failed, when set comment, "
                    "user_id:[%llu] comment_id:[%llu]", 
                    (unsigned long long)req->_user_id, (unsigned long long)req->_comment_id);
			return 0;
    }
*/
    if (_set_redis_list(comment_service._comment2post, PFX_COMMENT2POST, 
                req->_comment_id,req->_post_id) < 0 )
    {
        log_txt_err("save comment2post failed, comment_id[%llu] post_id[%llu]", 
                (unsigned long long)req->_comment_id, (unsigned long long)req->_post_id);
        return -1;
    }

    if (_set_redis_list(comment_service._post2comment, PFX_POST2COMMENT, 
                req->_post_id, req->_comment_id) < 0 )
    {
        log_txt_err("save post2comment failed, post_id[%llu] comment_id[%llu]", 
                (unsigned long long)req->_post_id, (unsigned long long)req->_comment_id);
        return -1;
    }



	/*    如果这条评论被同时转发了, 需要将comment_id和自生的_as_post_id写进 */
	/*  comment2post 索引中, 方便将针对当前comment_id再回复的内容, 关联到   */
	/*  所有 post 下去.                                                     */

    if (req->_as_post_id > 0 )
    {
        if (_set_redis_list(comment_service._comment2post, PFX_COMMENT2POST, 
                            req->_comment_id,req->_as_post_id) < 0 )
        {
            log_txt_err("save comment2post failed, comment_id[%llu] post_id[%llu]", 
                        (unsigned long long)req->_comment_id, (unsigned long long)req->_as_post_id);
            return -1;
        }
    }

    /*    如果该条评论是对 _parent_comment_id 的回复(reply), 需要取得       */
	/*  _parent_comment_id 的所有post_id，将当前 _comment_id 更新到这些     */
	/*  post_id 的 post2comment 索引中去.  另外, 将 _parent_comment_id      */
	/*  的 comment2comment 键值, 新增导入当前 _comment_id 的comment2comment */
	/*  索引表中.                                                           */

    if (req->_parent_comment_id > 0)
    {
        ret = _set_reply(req->_post_id, req->_parent_comment_id, req->_comment_id);
        if (ret < 0)
        {
            log_txt_err("set reply failed! post_comment_id:[%llu] comment_id:[%llu]", 
                        (unsigned long long)req->_parent_comment_id, (unsigned long long)req->_comment_id);
            return -1;
        }

        if (req->_parent_user_id <= 0)
        {
            log_txt_err("parent user_id was zero, when set reply, parent_comment_id:[%llu]", 
                        (unsigned long long)req->_parent_comment_id);
        }
        
        /*     由于针对 _root_user_id 的提醒, 统一放在更后的业务逻辑中实现, 为了避免  */
        /* _root_user_id 和 _parent_user_id 相同的情况下被重复通知, 此处排重.         */

        else if ( req->_user_id != req->_parent_user_id && 
                  req->_root_user_id != req->_parent_user_id )
        {
            if (_set_redis_list(comment_service._user2comment, PFX_USER2COMMENT, 
                                req->_parent_user_id, req->_comment_id) < 0 )
            {
				log_txt_warning("set user2comment failed, when set reply, "
                            "parent_comment_id:[%llu] parent_user_id:[%llu] comment_id:[%llu]", 
                            (unsigned long long)req->_parent_comment_id, 
                            (unsigned long long)req->_parent_user_id, 
                            (unsigned long long)req->_comment_id);
				return 0;
			}
            if (_set_push(comment_service._push, PUSH_SERVICE_TYPE_NEWCOMMENT, req->_parent_user_id, req->_user_id) < 0)
            {
                log_txt_warning("set push failed. parent_user_id:[%llu], comment_id:[%llu]",
                            (unsigned long long)req->_parent_user_id, (unsigned long long)req->_comment_id) ;
                return 0;
            }
		}
    }
    else
    {
		/*  如果 _post_id 是之前评论且转发来生成的, 查找 _upward_post_id.       */
		if (req->_post_equal_comment_id != 0 )
		{
            uint64_t _upward_post_ids[MAX_RET_POST_NUM] ;
            int elementSize = _get_redis_list(comment_service._comment2post, req->_post_equal_comment_id, 0, 0,
                                      PFX_COMMENT2POST, MAX_RET_COMMENT_NUM, _upward_post_ids) ;

			if ( elementSize < 0 )
                return -1 ;
            else if (elementSize == 0 )
				log_txt_warning("Find _upward_post_id failure.") ;

			/* 如果有中间环节的 _upward_post_id, 把当前 _comment_id 都写入它们  */
			/* 的 post2comment 索引中.                                          */
            for (int i=0; i<elementSize; i++)
            {
                /*  如果一篇 comment 被同时转发生成了一篇post, 那么它的 _comment_id   */
                /*  和它自身的 _as_post_id 的对应关系会被写进 post2comment 索引数据   */
                /*  中. 因此这里查找 _upward_post_ids 结果中需要排除掉 _as_post_id.   */

                if ( _upward_post_ids[i] == req->_post_id)
                    continue ;
                /*
                log_txt_info("_upward_post_ids[%d]=%llu", i, _upward_post_ids[i]) ;
                */
                if (_set_redis_list(comment_service._post2comment, PFX_POST2COMMENT, 
                                    _upward_post_ids[i],req->_comment_id) < 0 )
                {
                    log_txt_err("save post2comment failed, post_id[%llu] comment_id[%llu]", 
                        (unsigned long long)_upward_post_ids[i], (unsigned long long)req->_comment_id);
                    return -1;
                }
                if (_set_redis_list(comment_service._comment2post, PFX_COMMENT2POST, 
                                    req->_comment_id, _upward_post_ids[i]) < 0 )
                {
                    log_txt_err("save comment2post failed, comment_id[%llu] post_id[%llu]", 
                                    (unsigned long long)req->_comment_id, (unsigned long long)_upward_post_ids[i]);
                    return -1;
                }
            }
		}
	}

	/*   不管何种情况, 只要有评论产生, 都要通知 _root_user_id. 但是这个地方需要  */
    /* 注意, _root_user_id 虽然表示最原始文章的作者, 但是在IS模块中, 是将ref_pid */
    /* 对应的作者赋值给它, 因此当 _root_user_id 为零时并不表示一种错误.          */

	if (req->_root_user_id <= 0)
    {
        /*
		log_txt_err("root user_id was zero, when set reply, root_post_id:[%llu]", 
                    (unsigned long long)req->_post_id);
        */
	}
    else if ( req->_user_id != req->_root_user_id )
    {
        if ( _set_redis_list(comment_service._user2comment, PFX_USER2COMMENT,
                             req->_root_user_id, req->_comment_id) < 0 )
        {
			log_txt_err("set user2comment failed, when set comment, "
                        "root_user_id:[%llu] comment_id:[%llu]", 
                        (unsigned long long)req->_root_user_id, (unsigned long long)req->_comment_id);
			return 0;
		}
        if ( _set_push(comment_service._push, PUSH_SERVICE_TYPE_NEWCOMMENT, req->_root_user_id, req->_user_id) < 0 )
        {
            log_txt_err("set push failed. root_user_id:[%llu], comment_id:[%llu]",
                        (unsigned long long)req->_root_user_id, (unsigned long long)req->_comment_id) ;
            return 0;
        }
	}
	
	/*   如果当前post不是_root_post, 而是不断转发产生的, 也通知 _post_user_id.  */
    /*   此处需要排除2种情况下的重复情况. 一: 该 _post_user_id 和 _root_user_id */
    /*   相同; 二: 该 _post_user_id 和 被回复的 _parent_user_id 相同.           */

	if ( req->_post_user_id > 0 && 
         req->_post_user_id != req->_user_id && 
         req->_post_user_id != req->_root_user_id && 
         req->_post_user_id != req->_parent_user_id
       )
    {
        if ( _set_redis_list(comment_service._user2comment, PFX_USER2COMMENT,
                             req->_post_user_id, req->_comment_id) < 0 )
        {
			log_txt_err("set user2comment failed, when set comment, "
                        "post_user_id:[%llu] comment_id:[%llu]", 
                        (unsigned long long)req->_post_user_id, (unsigned long long)req->_comment_id);
			return 0;
		}
        if (_set_push(comment_service._push, PUSH_SERVICE_TYPE_NEWCOMMENT, req->_post_user_id, req->_user_id) < 0)
        {
            log_txt_err("set push failed. post_user_id:[%llu], comment_id:[%llu]",
                        (unsigned long long)req->_post_user_id, (unsigned long long)req->_comment_id) ;
            return 0;
        }
	}
    /* 如果这条评论没有被转发，则在评论中通知被at的用户. 这是因为, 如果该条评论  */
    /* 被转发而生成一篇post, 系统会通过 set_post() 方法通知at的用户. 没有必要通  */
    /* 知2次.                                                                    */

    if (req->_as_post_id == 0 && req->_at_user_num <= MAX_AT_USER_NUM) 
    {
        _at_users(req->_user_id, req->_comment_id, req->_at_user_num, req->_at_users);
    }


    rsp->_header.len = sizeof(cs2is_set_comment_t);
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = 0;
    
    return 0;
}

static int _set_product_comment(struct io_buff *buff)
{
    is2cs_set_product_cmt_t *req = (is2cs_set_product_cmt_t *)buff->rbuff;
    cs2is_set_comment_t *rsp = (cs2is_set_comment_t *)buff->wbuff;
    int ret = 0;

    if (req->_user_id == 0)
    {
        log_txt_err("is2cs_set_product_cmt: _user_id equals 0.");
        return -1;
    }
    if (req->_product_cmt_id == 0)
    {
        log_txt_err("is2cs_set_product_cmt: _comment_id equals 0.");
        return -1;
    }
    if (req->_product_id == 0)
    {
        log_txt_err("is2cs_set_product_cmt: _product_id equals 0.");
        return -1;
    }
    if (req->_product_user_id == 0)
    {
        log_txt_err("is2cs_set_product_cmt: _product_user_id equals 0.");
        return -1;
    }

    if (_set_redis_list(comment_service._product2comment, PFX_PRODUCT2COMMENT, 
                req->_product_id, req->_product_cmt_id) < 0 )
    {
        log_txt_err("save product2comment failed, product_id[%llu] comment_id[%llu]", 
                (unsigned long long)req->_product_id, (unsigned long long)req->_product_cmt_id);
        return -1;
    }

    if (req->_parent_comment_id > 0)
    {
        if (req->_parent_user_id <= 0)
        {
            log_txt_err("_parent_comment_id exists:[%llu], but _parent_user_id was zero.", 
                        (unsigned long long)req->_parent_comment_id);
            return -1 ;
        }
        
        /*    如果该评论是对 _parent_comment_id 的回复, 需将 _parent_comment_id */
	    /*  的 productcmt2cmt 键值, 新增导入至 _product_cmt_id 的productcmt2cmt */
	    /*  索引表中.                                                           */

        uint64_t tmp_comment_ids[MAX_CMT_NUM_PER_THREAD-1] ;
        int elementSize = _get_redis_list(comment_service._productcmt2comment, req->_parent_comment_id, 0, 0,
                PFX_PRODUCTCMT2CMT, MAX_CMT_NUM_PER_THREAD-1, tmp_comment_ids) ;
        if ( elementSize < 0 )
            return -1 ;

        char cmd[redis_cmd_len];
        char *ptr = cmd;
        int lft_len = sizeof(cmd);
        ret = snprintf(ptr, lft_len, "RPUSH %s%llu %llu", PFX_PRODUCTCMT2CMT, 
            (unsigned long long)req->_product_cmt_id, (unsigned long long)req->_parent_comment_id);
        lft_len -= ret;
        ptr += ret;

        for (int i=0; i<elementSize; i++)
        {
            ret = snprintf(ptr, lft_len, " %llu", (unsigned long long)tmp_comment_ids[i]);
            lft_len -= ret;
            ptr += ret;
            if (lft_len < 0)
            {
                break;
            }
        }
        if (lft_len < 0)
        {
            log_txt_err("build cmd failed, cmd-buff was full, buff-size:[%d],"
                "LPUSH element size:[%d]", (int)sizeof(cmd), elementSize);
            return -1;
        }
        
        svr_group_t *rds = sm_get_svr(comment_service._productcmt2comment, req->_product_cmt_id);
        if (rds == NULL)
        {
            log_txt_err("get %s server failed, comment_id:[%lluu]", 
                    PFX_PRODUCTCMT2CMT, (unsigned long long)req->_product_cmt_id);
            return -1;
        }
        
        redisReply *reply = (redisReply*)redisCommand((redisContext*)rds->_cur_conn, cmd);
        if (reply == NULL)
        {
            log_txt_err("execute redis command failed, command:[%s]", cmd);
            sm_reconnect(comment_service._productcmt2comment, rds);
            return -1;
        }
        freeReplyObject(reply);


        /*   更新 _parent_user_id 的索引数据, 并且进行推送通知.                 */
        /*   由于针对 _product_user_id 的提醒, 统一放在更后的业务逻辑中实现,    */
        /* 为了避免 _product_user_id 和 _parent_user_id 相同情况下被重复通知,   */
        /* 此处需要排重.                                                        */

        if ( req->_user_id != req->_parent_user_id && 
             req->_product_user_id != req->_parent_user_id )
        {
            if (_set_redis_list(comment_service._user2productcmt, PFX_USER2PRODUCTCMT, 
                                req->_parent_user_id, req->_product_cmt_id) < 0 )
            {
				log_txt_warning("set user2productcmt failed, when set_product_comment, "
                            "parent_comment_id:[%llu] parent_user_id:[%llu] comment_id:[%llu]", 
                            (unsigned long long)req->_parent_comment_id, 
                            (unsigned long long)req->_parent_user_id, 
                            (unsigned long long)req->_product_cmt_id);
				return 0;
			}
            if (_set_push(comment_service._push, PUSH_SERVICE_TYPE_NEW_PRODUCT_COMMENT, req->_parent_user_id, req->_user_id) < 0)
            {
                log_txt_warning("set push failed. parent_user_id:[%llu], comment_id:[%llu]",
                            (unsigned long long)req->_parent_user_id, (unsigned long long)req->_product_cmt_id) ;
                return 0;
            }
		}
    }

    /*   更新 _product_user_id 的索引数据, 并且进行推送通知.                   */

    if ( req->_user_id != req->_product_user_id )
    {
        if ( _set_redis_list(comment_service._user2productcmt, PFX_USER2PRODUCTCMT,
                             req->_product_user_id, req->_product_cmt_id) < 0 )
        {
			log_txt_err("set user2productcmt failed, when set_product_comment, "
                        "product_user_id:[%llu] comment_id:[%llu]", 
                        (unsigned long long)req->_product_user_id, (unsigned long long)req->_product_cmt_id);
			return 0;
		}
        if ( _set_push(comment_service._push, PUSH_SERVICE_TYPE_NEW_PRODUCT_COMMENT, req->_product_user_id, req->_user_id) < 0 )
        {
            log_txt_err("set push failed. product_user_id:[%llu], comment_id:[%llu]",
                        (unsigned long long)req->_product_user_id, (unsigned long long)req->_product_cmt_id) ;
            return 0;
        }
	}

    rsp->_header.len = sizeof(cs2is_set_comment_t);
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = 0;
    
    return 0;
}

static int _proc_get_comment(struct io_buff *buff)
{
    if ( _init_service(&comment_service._post2comment, g_setting._post2comment_conf_file) < 0) {
        return -1;
    }

    return _get_comment(buff, comment_service._post2comment, PFX_POST2COMMENT);
}

static int _proc_get_product_comment(struct io_buff *buff)
{
    if ( _init_service(&comment_service._product2comment, g_setting._product2comment_conf_file) < 0) {
        return -1;
    }

    return _get_comment(buff, comment_service._product2comment, PFX_PRODUCT2COMMENT);
}

/* 查看评论中的对话链条. 类似于雪球网的"查看对话"功能.   */

static int _proc_get_thread(struct io_buff *buff)
{
    if ( _init_service(&comment_service._comment2comment, g_setting._comment2comment_conf_file) < 0) {
        return -1;
    }

    return _get_comment(buff, comment_service._comment2comment, PFX_COMMENT2COMMENT);
}

static int _proc_get_thread_in_product(struct io_buff *buff)
{
    if ( _init_service(&comment_service._productcmt2comment, g_setting._productcmt2comment_conf_file) < 0) {
        return -1;
    }

    return _get_comment(buff, comment_service._productcmt2comment, PFX_PRODUCTCMT2CMT);
}

static int _proc_set_comment(struct io_buff *buff)
{
    if (_init_service(&comment_service._comment2post, g_setting._comment2post_conf_file) < 0 || 
        _init_service(&comment_service._post2comment, g_setting._post2comment_conf_file) < 0 ||
        _init_service(&comment_service._comment2comment, g_setting._comment2comment_conf_file) < 0 ||
        _init_service(&comment_service._user2comment, g_setting._user2comment_conf_file) < 0 || 
		/*
        _init_service(&comment_service._user_comment, g_setting._user_comment_conf_file) < 0 || 
		*/
        _init_service(&comment_service._push, g_setting._push_conf_file) < 0 ||
        _init_service(&comment_service._atme_cmt, g_setting._atme_cmt_conf_file) < 0
       )
    {
        return -1;
    }

    return _set_comment(buff);
}

static int _proc_set_product_comment(struct io_buff *buff)
{
    if (_init_service(&comment_service._product2comment, g_setting._product2comment_conf_file) < 0 || 
        _init_service(&comment_service._productcmt2comment, g_setting._productcmt2comment_conf_file) < 0 ||
        _init_service(&comment_service._user2productcmt, g_setting._user2productcmt_conf_file) < 0 || 
        _init_service(&comment_service._push, g_setting._push_conf_file) < 0
       )
    {
        return -1;
    }

    return _set_product_comment(buff);
}

static int _get_comment_cnt(struct io_buff *buff, svr_mgr_t *svr_handler, const char *pfx)
{
    if (svr_handler == NULL) {
        return -1;
    }

    is2cs_get_cnt_t *req = (is2cs_get_cnt_t *)buff->rbuff;
    cs2is_get_cnt_t *rsp = (cs2is_get_cnt_t *)buff->wbuff;

    svr_group_t *rds = sm_get_svr(svr_handler, req->_obj_id);
    if (rds == NULL) {
        log_txt_err("get %s server failed, input obj_id:[%llu]", pfx, (unsigned long long)req->_obj_id);
        return -1;
    }

    redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn, 
                                        "LLEN %s%llu", pfx, (unsigned long long)req->_obj_id);
    if (reply == NULL) {
        log_txt_err("execute redis command failed, command:[LLEN %s%llu]", pfx, (unsigned long long)req->_obj_id);
        sm_reconnect(svr_handler, rds);
        return -1;
    }
    
    rsp->_header.len = sizeof(cs2is_get_cnt_t);
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = 0;
    rsp->_comment_num = reply->integer;

    freeReplyObject(reply);

    return 0;
}

static int _get_comment_tome(struct io_buff *buff, svr_mgr_t *svr_handler, const char *pfx)
{
    if (svr_handler == NULL) {
        return -1;
    }

    is2cs_get_comment_tome_t *req = (is2cs_get_comment_tome_t *)buff->rbuff;
    cs2is_get_comment_tome_t *rsp = (cs2is_get_comment_tome_t *)buff->wbuff;
    
    int elementSize = _get_redis_list(svr_handler, req->_user_id, req->_start_idx, req->_req_num,
                                      pfx, MAX_RET_COMMENT_NUM, rsp->_comments) ;
    if ( elementSize < 0 )
        return -1 ;

    rsp->_comment_num = elementSize;

    rsp->_header.len = sizeof(cs2is_get_comment_tome_t)-sizeof(rsp->_comments)+rsp->_comment_num*sizeof(rsp->_comments[0]);
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = 0;

    return 0;
}

static int _del_comment(struct io_buff *buff)
{
    is2cs_del_comment_t *req = (is2cs_del_comment_t *)buff->rbuff;
    rsp_pack_t *rsp = (rsp_pack_t *)buff->wbuff;

    // 1. 获取comment关联的所有post，将评论从post下删除
    // comment2post获取所有post, 分别从对应post2comment下删除

    uint64_t post_ids[MAX_RET_POST_NUM];
    uint64_t cid = req->_comment_id;
    uint64_t uid = req->_user_id;
    int post_num = _get_redis_list(comment_service._comment2post,
            cid, 0, 0, PFX_COMMENT2POST,
            MAX_RET_COMMENT_NUM, post_ids);
    if (post_num > MAX_RET_POST_NUM) {
        log_txt_err("post num is large, %d", post_num);
        post_num = MAX_RET_POST_NUM;
    }

    for (int i = 0; i < post_num; ++i) {
        if (_redis_lrem(comment_service._post2comment, PFX_POST2COMMENT,
                post_ids[i], cid) == -1) {
            log_txt_err("redis lrem failed, key: %s%llu, value: %llu",
                    PFX_POST2COMMENT, post_ids[i], cid);
        }
    }

	/*
    int ret = _redis_lrem(comment_service._user_comment, PFX_USER_COMMENT,
            uid, cid);
    if (ret == -1) {
        log_txt_err("redis lrem failed, key: %s%llu, value: %llu",
                PFX_USER_COMMENT, uid, cid);
    }
*/
    // 2. 从被评论者的索引数据 user2comment 中删除.
    // parent_user_id, root_user_id, post_user_id
    
    int ret = _redis_lrem(comment_service._user2comment, PFX_USER2COMMENT,
            req->_post_user_id, cid);
    if (ret == -1) {
        log_txt_err("redis lrem failed, key: %s%llu, value: %llu",
                PFX_USER2COMMENT, req->_post_user_id, cid);
    }

    if (req->_parent_user_id > 0) {
        ret = _redis_lrem(comment_service._user2comment, PFX_USER2COMMENT,
                req->_parent_user_id, cid);
        if (ret == -1)
        {
            log_txt_err("redis lrem failed, key: %s%llu, value: %llu",
                    PFX_USER2COMMENT, req->_parent_user_id, cid);
        }
    }
    if (req->_root_user_id > 0) {
        ret = _redis_lrem(comment_service._user2comment, PFX_USER2COMMENT,
                req->_root_user_id, cid);
        if (ret == -1)
        {
            log_txt_err("redis lrem failed, key: %s%llu, value: %llu",
                    PFX_USER2COMMENT, req->_root_user_id, cid);
        }
    }

    // 3. 整体删除comment2post, comment2coment
    _redis_key_del(comment_service._comment2post, PFX_COMMENT2POST, cid);
    _redis_key_del(comment_service._comment2comment, PFX_COMMENT2COMMENT, cid);

    rsp->_header.len = sizeof(rsp_pack_t);
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = 0;
    return 0;
}

/* 文章作者删除评论时, 一开始采用的是和 _del_comment() 不同的策略, 以保证 */
/* 在从本文转发生成的新文下, 对应的原始comment依然可保留.                 */

static int _del_post_comment(struct io_buff *buff)
{
    is2cs_del_post_comment_t *req = (is2cs_del_post_comment_t *)buff->rbuff;
    rsp_pack_t *rsp = (rsp_pack_t *)buff->wbuff;

    uint64_t uid = req->_user_id;
    uint64_t pid = req->_post_id;
    uint64_t cid = req->_comment_id;

    // 从post2comment下删除
    int ret = _redis_lrem(comment_service._post2comment, PFX_POST2COMMENT,
            pid, cid);
    if (ret == -1 || ret == 0) {
        log_txt_err("redis lrem failed, key: %s%llu, value: %llu",
                PFX_POST2COMMENT, pid, cid);
        return -1;
    }

    // 从comment2post下删除
    ret = _redis_lrem(comment_service._comment2post, PFX_COMMENT2POST,
            cid, pid);
    if (ret == -1) {
        log_txt_err("redis lrem failed, key: %s%llu, value: %llu",
                PFX_COMMENT2POST, cid, pid);
    }

    // 从文章作者下删除 user2comment
    ret = _redis_lrem(comment_service._user2comment, PFX_USER2COMMENT,
            uid, cid);
    if (ret == -1) {
        log_txt_err("redis lrem failed, key: %s%llu, value: %llu",
                PFX_USER2COMMENT, uid, cid);
    }

    rsp->_header.len = sizeof(rsp_pack_t);
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = 0;
    return 0;
}

static int _del_product_comment(struct io_buff *buff)
{
    is2cs_del_product_cmt_t *req = (is2cs_del_product_cmt_t *)buff->rbuff;
    rsp_pack_t *rsp = (rsp_pack_t *)buff->wbuff;

    // 1. 将评论从 product_id 下删除.

    if (_redis_lrem(comment_service._product2comment, PFX_PRODUCT2COMMENT, 
                req->_product_id, req->_product_cmt_id) == -1)
    {
            log_txt_err("redis lrem failed, key: %s%llu, value: %llu",
                    PFX_PRODUCT2COMMENT, req->_product_id, req->_product_cmt_id);
    }

    // 2. 从被评论者的索引数据 user2comment 中删除.
    
    int ret = _redis_lrem(comment_service._user2productcmt, PFX_USER2PRODUCTCMT, 
            req->_product_user_id, req->_product_cmt_id);
    if (ret == -1)
    {
        log_txt_err("redis lrem failed, key: %s%llu, value: %llu",
                PFX_USER2PRODUCTCMT, req->_product_user_id, req->_product_cmt_id);
    }

    if (req->_parent_user_id > 0)
    {
        ret = _redis_lrem(comment_service._user2productcmt, PFX_USER2PRODUCTCMT, 
                req->_parent_user_id, req->_product_cmt_id);
        if (ret == -1)
        {
            log_txt_err("redis lrem failed, key: %s%llu, value: %llu",
                    PFX_USER2PRODUCTCMT, req->_parent_user_id, req->_product_cmt_id);
        }
    }

    // 3. 整体删除 productcmt2cmt
    _redis_key_del(comment_service._productcmt2comment, PFX_PRODUCTCMT2CMT, req->_product_cmt_id);

    rsp->_header.len = sizeof(rsp_pack_t);
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = 0;
    return 0;
}
/*
static int _get_comment_by_user(struct io_buff *buff, svr_mgr_t *svr_handler, const char *pfx)
{
    if (svr_handler == NULL) {
        return -1;
    }

    is2cs_get_comment_by_user_t *req = (is2cs_get_comment_by_user_t *)buff->rbuff;
    cs2is_get_comment_by_user_t *rsp = (cs2is_get_comment_by_user_t *)buff->wbuff;

    int elementSize = _get_redis_list(svr_handler, req->_user_id, req->_start_idx, req->_req_num,
            pfx, MAX_RET_COMMENT_NUM, rsp->_comments) ;
    if ( elementSize < 0 )
        return -1 ;

    rsp->_comment_num = elementSize;

    rsp->_header.len = sizeof(cs2is_get_comment_by_user_t)-sizeof(rsp->_comments)+rsp->_comment_num*sizeof(rsp->_comments[0]);
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = 0;

    return 0;
}
*/
static int _proc_get_cnt_by_post(struct io_buff *buff)
{
    if ( _init_service(&comment_service._post2comment, g_setting._post2comment_conf_file) < 0) {
        return -1;
    }

    return _get_comment_cnt(buff, comment_service._post2comment, PFX_POST2COMMENT);
}

static int _proc_get_cnt_by_comment(struct io_buff *buff)
{
    if ( _init_service(&comment_service._comment2comment, g_setting._comment2comment_conf_file) < 0) {
        return -1;
    }

    return _get_comment_cnt(buff, comment_service._comment2comment, PFX_COMMENT2COMMENT);
}

static int _proc_get_cnt_by_product(struct io_buff *buff)
{
    if ( _init_service(&comment_service._product2comment, g_setting._product2comment_conf_file) < 0) {
        return -1;
    }

    return _get_comment_cnt(buff, comment_service._product2comment, PFX_PRODUCT2COMMENT);
}

static int _proc_get_cnt_by_productcmt(struct io_buff *buff)
{
    if ( _init_service(&comment_service._productcmt2comment, g_setting._productcmt2comment_conf_file) < 0) {
        return -1;
    }

    return _get_comment_cnt(buff, comment_service._productcmt2comment, PFX_PRODUCTCMT2CMT);
}

static int _proc_get_comment_tome(struct io_buff *buff)
{
    if (_init_service(&comment_service._user2comment, g_setting._user2comment_conf_file) < 0) {
        return -1;
    }

    return _get_comment_tome(buff, comment_service._user2comment, PFX_USER2COMMENT);
}

static int _proc_get_productcmt_tome(struct io_buff *buff)
{
    if (_init_service(&comment_service._user2productcmt, g_setting._user2productcmt_conf_file) < 0) {
        return -1;
    }

    return _get_comment(buff, comment_service._user2productcmt, PFX_USER2PRODUCTCMT);
}

static int _proc_get_comment_tome_cnt(struct io_buff *buff)
{
    if (_init_service(&comment_service._user2comment, g_setting._user2comment_conf_file) < 0) {
        return -1;
    }

    return _get_comment_cnt(buff, comment_service._user2comment, PFX_USER2COMMENT);
}

static int _proc_get_productcmt_tome_cnt(struct io_buff *buff)
{
    if (_init_service(&comment_service._user2productcmt, g_setting._user2productcmt_conf_file) < 0) {
        return -1;
    }

    return _get_comment_cnt(buff, comment_service._user2productcmt, PFX_USER2PRODUCTCMT);
}
/*
static int _proc_get_comment_by_user(struct io_buff *buff)
{
    if (_init_service(&comment_service._user_comment, g_setting._user_comment_conf_file) < 0) {
        return -1;
    }

    return _get_comment_by_user(buff, comment_service._user_comment, PFX_USER_COMMENT);
}

static int _proc_get_comment_by_user_cnt(struct io_buff *buff)
{
    if (_init_service(&comment_service._user_comment, g_setting._user_comment_conf_file) < 0) {
        return -1;
    }

    return _get_comment_cnt(buff, comment_service._user_comment, PFX_USER_COMMENT);
}
*/
static int _proc_get_comment_atme(struct io_buff *buff)
{
    if (_init_service(&comment_service._atme_cmt, g_setting._atme_cmt_conf_file) < 0) {
        return -1;
    }

    return _get_comment(buff, comment_service._atme_cmt, PFX_ATME_CMT);
}

static int _proc_get_comment_atme_cnt(struct io_buff *buff)
{
    if (_init_service(&comment_service._atme_cmt, g_setting._atme_cmt_conf_file) < 0) {
        return -1;
    }

    return _get_comment_cnt(buff, comment_service._atme_cmt, PFX_ATME_CMT);
}

static int _proc_del_comment(struct io_buff *buff)
{
    if (_init_service(&comment_service._user2comment, g_setting._user2comment_conf_file) < 0 ||
        _init_service(&comment_service._comment2post, g_setting._comment2post_conf_file) < 0 ||
        _init_service(&comment_service._comment2comment, g_setting._comment2comment_conf_file) < 0 ||
		/*
        _init_service(&comment_service._user_comment, g_setting._user_comment_conf_file) < 0 ||
		*/
        _init_service(&comment_service._post2comment, g_setting._post2comment_conf_file) < 0
        ) {
        return -1;
    }
    return _del_comment(buff);
}

static int _proc_del_post_comment(struct io_buff *buff)
{
    if (_init_service(&comment_service._user2comment, g_setting._user2comment_conf_file) < 0 ||
        _init_service(&comment_service._comment2post, g_setting._comment2post_conf_file) < 0 ||
        _init_service(&comment_service._comment2comment, g_setting._comment2comment_conf_file) < 0 ||
        _init_service(&comment_service._post2comment, g_setting._post2comment_conf_file) < 0
        ) {
        return -1;
    }
    return _del_post_comment(buff);
}

static int _proc_del_product_comment(struct io_buff *buff)
{
    if (_init_service(&comment_service._product2comment, g_setting._product2comment_conf_file) < 0 ||
        _init_service(&comment_service._productcmt2comment, g_setting._productcmt2comment_conf_file) < 0 ||
        _init_service(&comment_service._user2productcmt, g_setting._user2productcmt_conf_file) < 0
       ) {
        return -1;
    }
    return _del_product_comment(buff);
}


#ifdef __cplusplus
extern "C" {
#endif

    // 初始化cs, 只会在主线程中调用一次
int cs_init(const char *conf_file)
{
    int ret = 0;
    ret = conf_init(conf_file);
    if (ret < 0) {
        return -1;
    }

    g_setting._post2comment_conf_file[0] = 0x00;
    read_conf_str("CS", "POST_TO_COMMENT_CONF_FILE", g_setting._post2comment_conf_file, 
            sizeof(g_setting._post2comment_conf_file), "");
    if (g_setting._post2comment_conf_file[0] == 0x00)
    {
        conf_uninit() ;
        return -2;
    }

    g_setting._comment2post_conf_file[0] = 0x00;
    read_conf_str("CS", "COMMENT_TO_POST_CONF_FILE", g_setting._comment2post_conf_file, 
            sizeof(g_setting._comment2post_conf_file), "");
    if (g_setting._comment2post_conf_file[0] == 0x00)
    {
        conf_uninit() ;
        return -3;
    }

    g_setting._comment2comment_conf_file[0] = 0x00;
    read_conf_str("CS", "COMMENT_TO_COMMENT_CONF_FILE", g_setting._comment2comment_conf_file, 
            sizeof(g_setting._comment2comment_conf_file), "");
    if (g_setting._comment2comment_conf_file[0] == 0x00)
    {
        conf_uninit() ;
        return -4;
    }

    g_setting._user2comment_conf_file[0] = 0x00;
    read_conf_str("CS", "USER_TO_COMMENT_CONF_FILE", g_setting._user2comment_conf_file, 
            sizeof(g_setting._user2comment_conf_file), "");
    if (g_setting._user2comment_conf_file[0] == 0x00)
    {
        conf_uninit() ;
        return -5;
    }
/*
    g_setting._user_comment_conf_file[0] = 0x00;
    read_conf_str("CS", "USER_COMMENT_CONF_FILE", g_setting._user_comment_conf_file, 
            sizeof(g_setting._user_comment_conf_file), "");
    if (g_setting._user_comment_conf_file[0] == 0x00)
    {
        conf_uninit() ;
        return -5;
    }
*/

    g_setting._push_conf_file[0] = 0x00;
    read_conf_str("CS", "PUSH_CONF_FILE", g_setting._push_conf_file, 
            sizeof(g_setting._push_conf_file), "");
    if (g_setting._push_conf_file[0] == 0x00)
    {
        conf_uninit() ;
        return -6;
    }

    g_setting._atme_cmt_conf_file[0] = 0x00;
    read_conf_str("CS", "ATME_CMT_CONF_FILE", g_setting._atme_cmt_conf_file, 
            sizeof(g_setting._atme_cmt_conf_file), "");
    if (g_setting._atme_cmt_conf_file[0] == 0x00)
    {
        conf_uninit() ;
        return -7;
    }
    
    g_setting._product2comment_conf_file[0] = 0x00;
    read_conf_str("CS", "PRODUCT_TO_COMMENT_CONF_FILE", g_setting._product2comment_conf_file, 
            sizeof(g_setting._product2comment_conf_file), "");
    if (g_setting._product2comment_conf_file[0] == 0x00)
    {
        conf_uninit() ;
        return -8;
    }
    
    g_setting._productcmt2comment_conf_file[0] = 0x00;
    read_conf_str("CS", "PRODUCT_COMMENT_TO_COMMENT_CONF_FILE", g_setting._productcmt2comment_conf_file, 
            sizeof(g_setting._productcmt2comment_conf_file), "");
    if (g_setting._productcmt2comment_conf_file[0] == 0x00)
    {
        conf_uninit() ;
        return -9;
    }
    
    g_setting._user2productcmt_conf_file[0] = 0x00;
    read_conf_str("CS", "USER_TO_PRODUCT_COMMENT_CONF_FILE", g_setting._user2productcmt_conf_file, 
            sizeof(g_setting._user2productcmt_conf_file), "");
    if (g_setting._user2productcmt_conf_file[0] == 0x00)
    {
        conf_uninit() ;
        return -10;
    }

    conf_uninit() ;
    return 0;
}

// 处理评论相关请求
int cs_proc(struct io_buff *buff)
{
    req_pack_t *req = (req_pack_t *)buff->rbuff;

    int ret = 0;
    switch (req->_header.cmd) {
        case CMD_GET_COMMENT_BY_POST:
            ret = _proc_get_comment(buff);
            break;
        case CMD_GET_COMMENT_BY_COMMENT:
            ret = _proc_get_thread(buff);
            break;
        case CMD_SET_COMMENT:
            ret = _proc_set_comment(buff);
            break;
        case CMD_GET_COMMENT_CNT_BY_POST:
            ret = _proc_get_cnt_by_post(buff);
            break;
        case CMD_GET_COMMENT_CNT_BY_COMMENT:
            ret = _proc_get_cnt_by_comment(buff);
            break;
        case CMD_GET_COMMENT_TOME:
            ret = _proc_get_comment_tome(buff);
            break;
        case CMD_GET_COMMENT_TOME_CNT:
            ret = _proc_get_comment_tome_cnt(buff);
            break;
        case CMD_GET_COMMENT_ATME:
            ret = _proc_get_comment_atme(buff);
            break;
        case CMD_GET_COMMENT_ATME_CNT:
            ret = _proc_get_comment_atme_cnt(buff);
            break;
			/*
        case CMD_GET_COMMENT_BY_USER:
            ret = _proc_get_comment_by_user(buff);
            break;
        case CMD_GET_COMMENT_BY_USER_CNT:
            ret = _proc_get_comment_by_user_cnt(buff);
            break;
			*/
        case CMD_DEL_COMMENT:
            ret = _proc_del_comment(buff);
            break;
        case CMD_DEL_POST_COMMENT:
            ret = _proc_del_post_comment(buff);
            break;

        /* 2016.07.06: 针对产品页的评论功能. */
        case CMD_GET_PRODUCT_COMMENT_BY_PRODUCT:
            ret = _proc_get_product_comment(buff);
            break;
        case CMD_GET_COMMENT_CNT_BY_PRODUCT:
            ret = _proc_get_cnt_by_product(buff);
            break;
        case CMD_GET_PRODUCT_COMMENT_BY_COMMENT:
            ret = _proc_get_thread_in_product(buff);
            break;
        case CMD_GET_COMMENT_CNT_BY_PRODUCT_COMMENT:
            ret = _proc_get_cnt_by_productcmt(buff);
            break;
        case CMD_SET_PRODUCT_COMMENT:
            ret = _proc_set_product_comment(buff);
            break;
        case CMD_GET_PRODUCT_COMMENT_TOME:
            ret= _proc_get_productcmt_tome(buff);
            break;
        case CMD_GET_PRODUCT_COMMENT_TOME_CNT:
            ret = _proc_get_productcmt_tome_cnt(buff);
            break;
        case CMD_DEL_RPODUCT_COMMENT:
            ret = _proc_del_product_comment(buff);
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

// 释放相关资源，只会在主线程中调用一次
int cs_uninit()
{
    if (comment_service._post2comment)
        sm_uninit(comment_service._post2comment);
    if (comment_service._comment2post)
        sm_uninit(comment_service._comment2post);
    if (comment_service._comment2comment)
        sm_uninit(comment_service._comment2comment);
    if (comment_service._user2comment)
        sm_uninit(comment_service._user2comment);
	/*
    if (comment_service._user_comment)
        sm_uninit(comment_service._user_comment);
	*/
    if (comment_service._atme_cmt)
        sm_uninit(comment_service._atme_cmt);
    if (comment_service._push)
        sm_uninit(comment_service._push);

    if (comment_service._product2comment)
        sm_uninit(comment_service._product2comment);
    if (comment_service._productcmt2comment)
        sm_uninit(comment_service._productcmt2comment);
    if (comment_service._user2productcmt)
        sm_uninit(comment_service._user2productcmt);

    return 0;
}

#ifdef __cplusplus
}
#endif
