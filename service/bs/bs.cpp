#include "buffer.h"
#include "interface.h" 
#include "conf.h"
#include "log.h"
#include "redis_handler.h"
#include "common_func.h"

/// 发送返回包，在框架中定义。
void dispatch_write(struct io_buff *buff);

/// bs配置信息
typedef struct {
    char _redis_host[MAX_SVR_HOST_LEN];
    short _redis_port;
} bs_setting_t;

/// 线程内部全局变量
__thread redis_service_t *g_rds;

/// 全局配置信息
static bs_setting_t g_setting;

static int _read_conf(const char *conf_file, bs_setting_t *setting)
{
    if (setting == NULL) {
        return -1;
    }

    int ret = 0;
    ret = conf_init(conf_file);
    if (ret < 0) {
        log_txt_err("initialize conf failed! conf[%s]", conf_file);
        return -1;
    }

    read_conf_str("BS", "REDIS_IP", setting->_redis_host, sizeof(setting->_redis_host), "");
	int _port ;
    read_conf_int("BS", "REDIS_PORT", &_port, 0);
    setting->_redis_port = _port ;
    if (setting->_redis_host[0] == 0x00 || setting->_redis_port == 0) {
        log_txt_err("read [BS]->[REDIS_IP] or [BS]->[REDIS_PORT] failed");
    
        conf_uninit();
        return -1;
    }

    conf_uninit();

    return 0;
}

// 获取post
static int _proc_get_post_cmd(struct io_buff *buff)
{
    int ret = 0;
    as2bs_get_post_t *req = (as2bs_get_post_t *) buff->rbuff;
    bs2as_get_post_t *rsp = (bs2as_get_post_t *) buff->wbuff;

    redis_get_post_req_t rds_req;
    rds_req._user_num = req->_user_num;
    rds_req._user_ids = req->_user_ids;
    rds_req._user_types = req->_user_types;
    rds_req._tag = req->_tag;

    rds_req._start_idx = 0 ;
    rds_req._req_num = req->_req_num;

    redis_get_post_rsp_t rds_rsp;
    ret = redis_get_post(g_rds, &rds_req, &rds_rsp);
    if (ret < 0) {
        log_txt_err("redis_get_post failed!");
        return -1;
    }

    // merge
    int in_list_x[MAX_RET_POST_NUM] ;
    int in_list_y[MAX_RET_POST_NUM] ;
    ret = multi_merge(rds_rsp._lists, rds_rsp._list_num, rsp->_posts, MAX_RET_POST_NUM,
                       in_list_x, in_list_y, req->_start_idx, req->_req_num);
    if (ret < 0) {
        log_txt_err("merge failed!")
        return -1;
    }

    for (int j=0; j<ret; j++)
    {
        rsp->_user_types[j] = req->_user_types[in_list_x[j]] ;
        rsp->_user_ids[j] = req->_user_ids[in_list_x[j]];
    }
    
    // pack ret
    rsp->_header.len = sizeof(bs2as_get_post_t) - sizeof(rsp->_posts) + sizeof(rsp->_posts[0]) * ret;
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence;
    rsp->_header.state = 0;
    rsp->_post_num = ret;

    log_txt_info("finished one get-post-request, return-pack-length:%d", rsp->_header.len);

    return 0;
}

// added by Radio: 获取post list
static int _proc_get_post_by_page_cmd(struct io_buff *buff)
{
    int ret = 0;
    as2bs_get_post_t *req = (as2bs_get_post_t *) buff->rbuff;
    bs2as_get_post_t *rsp = (bs2as_get_post_t *) buff->wbuff;

    redis_get_post_req_t rds_req;
    rds_req._user_num = req->_user_num;
    rds_req._user_ids = req->_user_ids;
    rds_req._tag = req->_tag;

    rds_req._start_idx = req->_start_idx;
    rds_req._req_num = req->_req_num;

    redis_get_post_rsp_t rds_rsp;
    ret = redis_get_post_by_page(g_rds, &rds_req, &rds_rsp);
    if (ret < 0) {
        log_txt_err("redis_get_post failed!");
        return -1;
    }

    //for debug
    /*
    log_txt_err("rds_rsp._list_num=%d", rds_rsp._list_num) ;
    for (int i=0; i<rds_rsp._list_num; i++)
    {
        int j=0 ;
        while (rds_rsp._lists[i][j] != 0)
        {
            log_txt_err("rds_rsp._lists[%d][%d]=%llu",i,j,rds_rsp._lists[i][j]) ;
            j++ ;
        }
    }
    */

    int list_idx = rds_rsp._list_num - 1; 
    int actual_post_num = 0; 
    while(rds_rsp._lists[list_idx][actual_post_num] != 0) {
        rsp->_posts[actual_post_num] = rds_rsp._lists[list_idx][actual_post_num];
        rsp->_user_types[actual_post_num] = req->_user_types[list_idx];
        rsp->_user_ids[actual_post_num] = req->_user_ids[list_idx];
        actual_post_num++;
    }    
    
    //for debug
    /*
    for (int j=0; j<actual_post_num; j++)
    {
        log_txt_err("post id[%d]=%llu", j, rsp->_posts[j]) ;
        log_txt_err("user_types[%d]=%d", j, rsp->_user_types[j]) ;
        log_txt_err("user_ids[%d]=%llu", j, rsp->_user_ids[j]) ;
    }
    */

    // pack ret
    rsp->_header.len = sizeof(bs2as_get_post_t) - sizeof(rsp->_posts) + sizeof(rsp->_posts[0]) * actual_post_num;
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence;
    rsp->_header.state = 0;
    rsp->_post_num = actual_post_num;

    return 0;
}

// 保存post
static int _proc_set_post_cmd(struct io_buff *buff)
{
    int ret = 0;
    as2bs_set_post_t *req = (as2bs_set_post_t *) buff->rbuff;
    bs2as_set_post_t *rsp = (bs2as_set_post_t *) buff->wbuff;

    redis_set_post_req_t wp_req;
    wp_req._post_id = req->_post_id;
    wp_req._pair_cnt = req->_pair_cnt;
    wp_req._user_ids = req->_user_ids;

    for (int i = 0; i < req->_pair_cnt; i++) {
        //wp_req._tags = req->_tags;
        wp_req._tags[i] = (char *)(req->_data) + req->_tags[i];
    }

	/*  区分要加入索引, 还是从索引中脱钩. */
	if (req->_set_post_user_type == (int) SET_POST_USERID_SET) {
		ret = redis_set_post(g_rds, &wp_req);
		if (ret < 0) {
			log_txt_err("redis_set_post failed!");
			return -1;
		}
	}
	else if (req->_set_post_user_type == (int) SET_POST_USERID_REMOVE) {
		ret = redis_remove_post(g_rds, &wp_req);
		if (ret < 0) {
			log_txt_err("redis_remove_post failed!");
			return -1;
		}
	}
	else {
		log_txt_err("_set_post_user_type error: %d", req->_set_post_user_type) ;
		return -1 ;
	}

    // pack ret
    rsp->_header.len = sizeof(bs2as_set_post_t);
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence;
    rsp->_header.state = 0;

    return 0;
}

// 获取post
static int _proc_get_post_cnt_cmd(struct io_buff *buff)
{
    as2bs_get_post_cnt_t *req = (as2bs_get_post_cnt_t *) buff->rbuff;
    bs2as_get_post_cnt_t *rsp = (bs2as_get_post_cnt_t *) buff->wbuff;

    // 构造请求参数
    redis_get_post_cnt_req_t rds_req;
    rds_req._user_num = req->_user_num;
    rds_req._user_ids = req->_user_ids;
    rds_req._tag = req->_tag;

    int post_cnt;
    post_cnt = redis_get_post_cnt(g_rds, &rds_req);
    if (post_cnt < 0) {
        log_txt_err("redis_get_post failed!");
        return -1;
    }
    // pack ret
    rsp->_header.len = sizeof(bs2as_get_post_cnt_t);
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence;
    rsp->_header.state = 0;
    rsp->_post_num = post_cnt;

    return 0;
}

static int _build_failed_pack(struct io_buff *buff)
{
    req_pack_t *req = (req_pack_t *) buff->rbuff;
    rsp_pack_t *rsp = (rsp_pack_t *) buff->wbuff;

    // pack response
    rsp->_header.len = sizeof(rsp_pack_t);
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence;
    rsp->_header.state = -1;

    return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

int bs_init(const char *conf_file)
{
    int ret = 0;

    ret = _read_conf(conf_file, &g_setting);
    if (ret < 0) {
        return -1;
    }

    return 0;
}

int bs_proc(struct io_buff *buff)
{
    if (g_rds == NULL) {
        g_rds = redis_create(g_setting._redis_host, g_setting._redis_port);
        if (g_rds == NULL)
        {
            log_txt_err("initialize redis failed! thread_id:%lu", pthread_self());
            return -1;
        }
    }

    req_pack_t *req = (req_pack_t *) buff->rbuff;

    int ret = 0;
    switch (req->_header.cmd) {
        case CMD_GET_POST:
            ret = _proc_get_post_cmd(buff);
            break;
        case CMD_GET_POST_BY_PAGE:
            ret = _proc_get_post_by_page_cmd(buff);
            break;
        case CMD_SET_POST:
            ret = _proc_set_post_cmd(buff);
            break;
        case CMD_GET_POST_CNT:
            ret = _proc_get_post_cnt_cmd(buff);
            break;
        default:
            log_txt_err("unkown command number:[%d]", req->_header.cmd);
            ret = -1;
            break;
    }

    if (ret < 0) {
        _build_failed_pack(buff);
    }

    return ret;
}

int bs_uninit()
{
    redis_close(g_rds);
    g_rds = NULL;
	return 0 ;
}

#ifdef __cplusplus
}
#endif

