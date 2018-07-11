#include "company_match.h"

#include <algorithm>
#include <inttypes.h>
#include <set>
#include <string.h>

#include "StringUtil.h"
#include "common.h"
#include "conf.h"
#include "hiredis.h"
#include "interface.h"
#include "log.h"
#include "server_manager.h"

#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"

#define SIMILARITY_THRESHOLD 2

#define COMPANY_STR_MAX_LEN                                                    \
    (4 * (20 + 1) + 2 + 1 + MAX_SCENICS_NUM_PER_COMPANY * (20 + 1))

typedef struct {
    char _company2scenic_conf_file[MAX_FILE_PATH_LEN];
    char _user2company_conf_file[MAX_FILE_PATH_LEN];
    char _scenic2company_conf_file[MAX_FILE_PATH_LEN];
    char _company2company_conf_file[MAX_FILE_PATH_LEN];
    char _company_conf_file[MAX_FILE_PATH_LEN];

    char _push_conf_file[MAX_FILE_PATH_LEN];
    char _company_match_queue_conf_file[MAX_FILE_PATH_LEN];

} company_conf_t;

typedef struct {
    svr_mgr_t *_company2scenic;
    svr_mgr_t *_user2company; // 用户发布的结伴需求.
    svr_mgr_t *_scenic2company; // 保存景点下所有结伴id, 包括过期和撤销的.
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

company_conf_t g_setting;
company_service_t company_service = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};

const int redis_cmd_len = 1024 * 200;

static int _create_redis_conn(char *host, short port, void **conn) {
    redisContext *c = redisConnect(host, port);
    if (c == NULL) {
        log_txt_err("redisConnect failed, host[%s], port[%d]", host, port);
        return -1;
    }
    if (c->err) {
        log_txt_err("redisConnect failed, host[%s], port[%d], msg:[%s]\n", host,
                port, c->errstr);
        return -1;
    }
    *conn = c;
    return 0;
}

static int _destroy_redis_conn(void *conn) {
    redisFree((redisContext *)conn);
    return 0;
}

static int _init_service(svr_mgr_t **svr, char *conf) {
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
/*
   static int _redis_set_string(svr_mgr_t *svr_handler, const char *pfx,
   uint64_t key, char *buf) {
   if (svr_handler == NULL)
   return -1;
   svr_group_t *rds = sm_get_svr(svr_handler, key);
   if (rds == NULL) {
   log_txt_err("get %s server failed, key:[%" PRIu64 "]", pfx, key);
   return -1;
   }

   redisReply *reply = (redisReply *)redisCommand(
   (redisContext *)rds->_cur_conn, "SET %s%" PRIu64 " %s", pfx, key, buf);
   if (reply == NULL) {
   log_txt_err("execute redis command failed: [SET %s%" PRIu64 "]", pfx, key);
   sm_reconnect(svr_handler, rds);
   return -1;
   }
   freeReplyObject(reply);
   return 0;
   }
   */

// redis 读字符串
static int _redis_get_string(svr_mgr_t *svr_handler, const char *pfx,
        uint64_t key, char *buf, int buf_len) {
    if (svr_handler == NULL)
        return -1;

    svr_group_t *rds = sm_get_svr(svr_handler, key);
    if (rds == NULL) {
        log_txt_err("get %s server failed, key:[%" PRIu64 "]", pfx, key);
        return -1;
    }

    redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn,
            "GET %s%" PRIu64 "", pfx, key);
    if (reply == NULL) {
        log_txt_err("execute redis command failed: [GET %s%" PRIu64 "]", pfx, key);
        sm_reconnect(svr_handler, rds);
        return -1;
    }
    if (reply->type != REDIS_REPLY_STRING) {
        if (reply->type == REDIS_REPLY_ERROR) {
            log_txt_err("redis GET string exception: %s", reply->str);
        }
        freeReplyObject(reply);
        return -1;
    }
    if (reply->len > buf_len) {
        log_txt_err("redis GET string invalid data");
        freeReplyObject(reply);
        return -1;
    }
    if (reply->len > 0) {
        memcpy(buf, reply->str, reply->len);
    }
    freeReplyObject(reply);
    return reply->len;
}

static int _get_redis_list(svr_mgr_t *svr_handler, uint64_t key, int start_idx,
                           int req_num, const char *pfx, int ret_size, uint64_t *ret_list)
{
    if (svr_handler == NULL)
        return -1;

    svr_group_t *rds = sm_get_svr(svr_handler, key);
    if (rds == NULL) {
        log_txt_err("get %s server failed, key:[%" PRIu64 "]", pfx, key);
        return -1;
    }

    redisReply *reply = (redisReply *)redisCommand( (redisContext *)rds->_cur_conn, 
                                                    "LRANGE %s%" PRIu64 " %d %d", pfx, key,
                                                    start_idx, start_idx + req_num - 1);
    if (reply == NULL)
    {
        log_txt_err("execute redis command failed: [LRANGE %s%" PRIu64 " %d %d]",
                pfx, key, start_idx, start_idx + req_num - 1);
        sm_reconnect(svr_handler, rds);
        return -1;
    }

    int elementSize = 0;
    for (size_t i = 0; i < reply->elements; i++) {
        if (reply->element[i]->type == REDIS_REPLY_INTEGER)
            ret_list[i] = (uint64_t)(reply->element[i]->integer);
        else if (reply->element[i]->type == REDIS_REPLY_STRING)
            sscanf(reply->element[i]->str, "%" PRIu64 "", &(ret_list[i]));
        else {
            log_txt_err("redis query success, result type incorrect.");
            break;
        }
        if (++elementSize >= ret_size)
        {
            //log_txt_err("return list was full, size:[%d] need_size:[%lu]", ret_size,reply->elements);
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

static int _set_redis_list(svr_mgr_t *svr_handler, const char *pfx,
        uint64_t key, uint64_t value) {
    if (svr_handler == NULL) {
        return -1;
    }

    svr_group_t *rds = sm_get_svr(svr_handler, key);
    if (rds == NULL) {
        log_txt_err("get %s server failed, key:[%" PRIu64 "]", pfx, key);
        return -1;
    }

    redisReply *reply = (redisReply *)redisCommand(
            (redisContext *)rds->_cur_conn, "LPUSH %s%" PRIu64 " %" PRIu64 "", pfx,
            key, value);
    if (reply == NULL) {
        log_txt_err("execute redis command failed: [LPUSH %s%" PRIu64 " %" PRIu64
                "]",
                pfx, key, value);
        sm_reconnect(svr_handler, rds);
        return -1;
    }

    freeReplyObject(reply);
    return 0;
}

/* rpop a string from redis list. */
static int _rpop_string_redis_list(svr_mgr_t *svr_handler, const char *pfx, uint64_t key, std::string &result)
{
    if (svr_handler == NULL)
        return -1;

    svr_group_t *rds = sm_get_svr(svr_handler, key);
    if (rds == NULL) {
        log_txt_err("get %s server failed, key:[%" PRIu64 "]", pfx, key);
        return -1;
    }

    redisReply *reply = (redisReply *)redisCommand((redisContext *)rds->_cur_conn,
                                                   "RPOP %s%" PRIu64, pfx, key);
    if (reply == NULL)
    {
        log_txt_err("execute redis command failed: [RPOP %s%" PRIu64 "]", pfx, key);
        sm_reconnect(svr_handler, rds);
        return -1;
    }

    int pop_size = 0 ;

    if (reply->type == REDIS_REPLY_STRING)
    {
        result = std::string(reply->str);
        pop_size = 1 ;
    }
    else if (reply->type != REDIS_REPLY_NIL)
    {
        log_txt_err("pop result type not string. return 0.");
    }

    freeReplyObject(reply);
    return pop_size;
}

static int get_company_info(uint64_t company_id, company_info_t *company)
{
    char company_str_arr[COMPANY_STR_MAX_LEN];
    memset(company_str_arr, 0, sizeof(char) * COMPANY_STR_MAX_LEN);
    int ret = _redis_get_string(company_service._company, PFX_COMPANY, company_id,
                                company_str_arr, COMPANY_STR_MAX_LEN);

    if (ret < 0)
    {
        /* 过期失效. */
        // log_txt_info("Tried to get company, but failed.");
        return ret;
    }

    std::string company_str(company_str_arr);
    StrTokenizer tokens(company_str, ";");
    if (tokens.count_tokens() <= 5)
    {
        log_txt_err("Company info read out, but inconsistent.");
        return -2;
    }

    company->user_id = StringUtil::StrToUint64(tokens.token(0));
    company->company_id = StringUtil::StrToUint64(tokens.token(1));
    company->start_date = StringUtil::StrToUint64(tokens.token(2));
    company->end_date = StringUtil::StrToUint64(tokens.token(3));
    company->scenic_num = StringUtil::StrToUint64(tokens.token(4));
    for (int i = 0; i < company->scenic_num; i++)
        company->scenics[i] = StringUtil::StrToUint64(tokens.token(5 + i));

    return ret;
}

/* COMPANY 模块, 当有推荐结伴需求的时候, 向Trans模块推送通知. */

static int _set_push(svr_mgr_t *svr_handler, int push_service_type,
                     uint64_t user_id, uint64_t action_uid, uint64_t company_id,
                     uint64_t dest_company_id)
{
    if (push_service_type != PUSH_SERVICE_TYPE_NEW_COMPANY) {
        log_txt_err("push_service_type not valid: %d", push_service_type);
        return -1;
    }

    /* 获取 push 句柄 */
    svr_group_t *rds = sm_get_svr(svr_handler, user_id);
    if (rds == NULL) {
        log_txt_err("get %s server failed, user_id:[%" PRIu64 "]", PFX_PUSH,
                user_id);
        return -1;
    }

    char cmd[redis_cmd_len];
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd),
            "LPUSH %s {\"type\":%d,\"content\":{\"uid\":\"%" PRIu64 "\","
            "\"action_uid\":\"%" PRIu64 "\",\"company_id\":\"%" PRIu64 "\","
            "\"dest_company_id\":\"%" PRIu64 "\"}}",
            PFX_PUSH, push_service_type, user_id, action_uid, company_id,
            dest_company_id);

    /* 写入 push 消息队列 */
    redisReply *reply =
        (redisReply *)redisCommand((redisContext *)rds->_cur_conn, cmd);
    if (reply == NULL) {
        log_txt_err("execute redis command faild, command: %s", cmd);
        sm_reconnect(company_service._push, rds);
        return -1;
    }

    freeReplyObject(reply);
    return 0;
}

/* 返回两个结伴需求的相似度. 满分以10分计算.                               */

/* 设定1个标准, 双方覆盖3个景点, 并且日期重合3天, 便可拿到满分10分. 以此   */
/* 标准下推.                                                               */

static int company_similarity(company_info_t *lhs, company_info_t *rhs)
{
    /* 如果2个company 的日期跨度没有重合, 则相似度为0, 直接返回. */

    uint64_t late_start_date = std::max(lhs->start_date, rhs->start_date);
    uint64_t early_end_date = std::min(lhs->end_date, rhs->end_date);
    if (late_start_date > early_end_date)
        return 0;

    /* 日期重合1天, 贡献得分1; 日期重合2天时, 贡献得分2;          */
    /* 日期重合3天以上时, 贡献得分3.                              */

    /* 计算重合日期数时, 不对入参做假设, 它们的值可能没对准日期.  */
    /* 但是我们需要日期整数值.                                    */

    const uint64_t SECONDS_PER_DAY = 86400;
    uint64_t common_date_stamp = early_end_date - late_start_date;
    uint64_t common_date = common_date_stamp / SECONDS_PER_DAY;
    if (common_date_stamp % SECONDS_PER_DAY != 0)
        common_date += 1;

    if (common_date > 3)
        common_date = 3;

    int similarity = (common_date * 1);

    /* 把双方都覆盖的景点给找出来.                                */

    std::vector<uint64_t> common_scenics;
    for (int left_idx = 0; left_idx < lhs->scenic_num; left_idx++) {
        for (int right_idx = 0; right_idx < rhs->scenic_num; right_idx++) {
            if (rhs->scenics[right_idx] == lhs->scenics[left_idx]) {
                common_scenics.push_back(lhs->scenics[left_idx]);
                break;
            }
        }
    }

    /* 如果2个company 的覆盖景点没有重合, 则相似度为0, 直接返回.  */
    /* 共同景点数为1时, 贡献得分2; 共同景点数为2时, 贡献得分4;    */
    /* 共同景点数达到3时, 贡献得分7;                              */

    int common_scenics_size = (int)common_scenics.size();
    if (common_scenics_size == 0)
        return 0;
    else if (common_scenics_size == 1)
        similarity += 2;
    else if (common_scenics_size == 2)
        similarity += 4;
    else
        similarity += 7;

    return similarity;
}

struct CompanyResult {
    CompanyResult() {}
    CompanyResult(uint64_t user_id, uint64_t company_id, int similarity)
        : _user_id(user_id), _company_id(company_id), _similarity(similarity) {}
    uint64_t _user_id;
    uint64_t _company_id;
    int _similarity;
};

bool compare_result(const CompanyResult &a, const CompanyResult &b) {
    return a._similarity < b._similarity;
}

int execute_match(company_info_t *company, int scenic_num, 
                  uint64_t *covered_scenics, int matched_company_num,
                  uint64_t *matched_companies)
{
    if (scenic_num < 1 || (scenic_num >= 1 && covered_scenics == NULL)) {
        log_txt_err("scenic_num: %d", scenic_num);
        return -1;
    }
    if (matched_company_num > 0 && matched_companies == NULL) {
        log_txt_err("matched_company_num: %d", matched_company_num);
        return -1;
    }

    /* 这里保存当前结伴已经尝试匹配过的对方结伴ID.   */
    std::set<uint64_t> compared_company_set;
    for (int i = 0; i < matched_company_num; i++) {
        compared_company_set.insert(matched_companies[i]);
    }

    /* 这里保存当前结伴已经匹配成功过的对方用户.     */
    std::set<uint64_t> matched_user_set;

    /* 这里保存所有和当前用户历史结伴成功匹配的结伴. */
    std::set<uint64_t> history_matched_company_set;
    bool history_matched_company_fetched = false;

    std::vector<CompanyResult> results;

    for (int i = 0; i < scenic_num; i++)
    {
        /* 对每个旅行地, 只尝试匹配 100 个对方结伴. */

        const int company_depth_of_scenic = 100;
        uint64_t companies_of_scenic[company_depth_of_scenic];

        int ret = _get_redis_list(company_service._scenic2company, covered_scenics[i], 0, 
                                  company_depth_of_scenic, PFX_SCENIC2COMPANY,
                                  company_depth_of_scenic, companies_of_scenic);

        for (int j = 0; j < ret; j++)
        {
            uint64_t tmp_other_id = companies_of_scenic[j];

            /* 如果是同一个company, 或尝试匹配过, 则跳过;                        */
            if (tmp_other_id == company->company_id ||
                compared_company_set.find(tmp_other_id) != compared_company_set.end())
            {
                continue;
            }
            compared_company_set.insert(tmp_other_id);

            /* 对方结伴必须在有效期内.   */

            company_info_t other;
            int other_ret = get_company_info(tmp_other_id, &other);

            uint64_t now = time(NULL);
            if (other_ret <= 0 || other.user_id == company->user_id ||
                    other.end_date < now)
                continue;

            /* 如果对方用户的某结伴已被本次结伴成功匹配, 则忽略对方用户的其他结伴.  */
            if (matched_user_set.find(other.user_id) != matched_user_set.end())
                continue;

            /* 收集当前用户历史结伴的匹配结伴, 保存进 set 数据结构里. 只会执行一次. */

            /* 如果对方结伴已经是当前用户历史结伴的匹配结伴, 则不再重复给对方推荐.  */

            if (history_matched_company_fetched == false)
            {
                /* 这里保存当前用户发布过的至多4个历史结伴.      */
                /* 获取时跳过第一个: 当前结伴.                   */
                uint64_t self_top_four_companies[4];
                int self_history_num =
                    _get_redis_list(company_service._user2company, company->user_id, 1,
                            4, PFX_USER2COMPANY, 4, self_top_four_companies);

                for (int k = 0; k < self_history_num; k++)
                {
                    uint64_t history_matched_company_one[MAX_RET_COMPANY_NUM];
                    int history_matched_company_num_one = _get_redis_list(
                            company_service._company2company, self_top_four_companies[k], 0,
                            MAX_RET_COMPANY_NUM, PFX_COMPANY2COMPANY, MAX_RET_COMPANY_NUM,
                            history_matched_company_one);

                    for (int l = 0; l < history_matched_company_num_one; l++)
                        history_matched_company_set.insert(history_matched_company_one[l]);
                }

                history_matched_company_fetched = true;
            }
            if (history_matched_company_set.find(other.company_id) != history_matched_company_set.end())
                continue;

            /* 请注意, 能够进入这个函数的2个结伴需求, 一定拥有了至少1个共同的景点了. */
            /* 并且, 1个是新发布的自己, 1个依然处于有效状态的. 相似度分数必须大于2.  */

            int similarity = company_similarity(company, &other);
            if (similarity > SIMILARITY_THRESHOLD)
            {
                results.push_back( CompanyResult(other.user_id, other.company_id, similarity) );
                matched_user_set.insert(other.user_id);
            }
        }
    }

    int results_num = (int)results.size();
    if ( results_num <= 0 )
        return 0 ;
    
    std::sort(results.begin(), results.end(), compare_result);

    /* 要保证左边的结果是相似度最高的. 由于 _set_redis_list 从左插入,
     * 所以按相似度值, 从小到大插入. */

    /* 推荐成功后是否要发送私信通知, 根据存量的推荐结伴数, 来动态决定. */

    int existed_num = 0 ;
    bool ready_to_notify = false ;

    for (int i = 0; i < results_num; ++i)
    {
        /* 将结果写入自己company2company */
        _set_redis_list(company_service._company2company, PFX_COMPANY2COMPANY, company->company_id, results[i]._company_id);

        /* 
         * 如果对方结伴, 只匹配到了40个以下的推荐结伴, 那么继续向对方推荐.
         * 青驿认为, 已经推荐了40个相关结伴, 发起人可以从中联系上; 再多的推荐,
         * 只会陷入选择困难症, 因此不再推荐.
         *
         */
       
        existed_num = _get_redis_list_cnt(company_service._company2company,PFX_COMPANY2COMPANY,results[i]._company_id);
        if ( existed_num > 40)
            continue ;

        /* 将结果写入对方company2company */
        _set_redis_list(company_service._company2company, PFX_COMPANY2COMPANY, results[i]._company_id, company->company_id);
     
        ready_to_notify = false ;

        if ( existed_num <= 1 )
            ready_to_notify = true ;
        else if ( existed_num <= 10 )
        {
            if ( existed_num % 5 == 0 )
                ready_to_notify = true ;
        }
        else if ( existed_num <= 40 )
        {
            if ( existed_num % 10 == 0 )
                ready_to_notify = true ;
        }

        if ( ready_to_notify )
        {
            _set_push(company_service._push, PUSH_SERVICE_TYPE_NEW_COMPANY, results[i]._user_id, company->user_id, results[i]._company_id,company->company_id);
        }

    }

    existed_num =  _get_redis_list_cnt(company_service._company2company,PFX_COMPANY2COMPANY,company->company_id)-results_num ;
    ready_to_notify = false ;

    if ( existed_num <= 10 )
        ready_to_notify = true ;
    else if ( existed_num <= 20 )
    {
        if ( results_num >= 5)
            ready_to_notify = true ;
    }
    else if ( existed_num <= 60 )
    {
        if ( results_num >= 10)
            ready_to_notify = true ;
    }

    if ( ready_to_notify )
    {
        /* set push 中采用相似度最高的结伴用户为 action_uid */
        _set_push(company_service._push, PUSH_SERVICE_TYPE_NEW_COMPANY,
                company->user_id, results[results_num - 1]._user_id,
                company->company_id, results[results_num - 1]._company_id);
    }

    return 0;
}

int string2company(const char *str, company_info_t *company, int *match_type)
{
    rapidjson::Document d;
    if (d.Parse(str).HasParseError()) {
        log_txt_err("[string2company] invalid json company task string, %s", str);
        return -1;
    }
    *match_type = d["match_type"].GetInt();
    company->company_id = d["company_id"].GetUint64();
    company->user_id = d["user_id"].GetUint64();
    company->start_date = d["start_date"].GetUint64();
    company->end_date = d["end_date"].GetUint64();
    company->scenic_num = d["scenic_num"].GetInt();
    rapidjson::Value &scenics = d["scenics"];
    for (int i = 0; i < company->scenic_num; ++i)
    {
        company->scenics[i] = scenics[i].GetUint64();
    }
    return 0;
}

int _pop_one_company(company_info_t *company, int *match_type)
{
    std::string result;

    int ret = _rpop_string_redis_list(company_service._company_match_queue,
                                      PFX_COMPANY_MATCH_QUEUE, 0, result);
    if (ret <= 0)
    {
        return 0;
    }

    if (string2company(result.c_str(), company, match_type) != 0)
    {
        return 0;
    }
    return 1;
}

int company_match_one()
{
    company_info_t company;
    int match_type = 0;
    if (_pop_one_company(&company, &match_type) == 0)
        return 0;
    
    if (match_type == COMPANY_MATCH_TYPE_NEW) 
    {
        execute_match(&company, company.scenic_num, company.scenics, 0, NULL);
    }
    else
    {
        uint64_t matched_companies[MAX_RET_COMPANY_NUM];
        int matched_company_num = _get_redis_list(company_service._company2company, company.company_id,
                                                  0, MAX_RET_COMPANY_NUM, PFX_COMPANY2COMPANY, 
                                                  MAX_RET_COMPANY_NUM, matched_companies);

        if (matched_company_num < 0)
            matched_company_num = 0 ;

        execute_match(&company, company.scenic_num, company.scenics, matched_company_num, matched_companies);
    }

    log_txt_info("match company: %" PRIu64 ", type:%d", company.company_id, match_type);
    return 1;
}

int company_match_init(const char *conf_file) {

    int ret = conf_init(conf_file);
    if (ret < 0) {
        return -1;
    }

    g_setting._company2scenic_conf_file[0] = 0x00;
    read_conf_str("COMPANY", "COMPANY_TO_SCENIC_CONF_FILE",
            g_setting._company2scenic_conf_file,
            sizeof(g_setting._company2scenic_conf_file), "");
    if (g_setting._company2scenic_conf_file[0] == 0x00) {
        conf_uninit();
        return -2;
    }

    g_setting._user2company_conf_file[0] = 0x00;
    read_conf_str("COMPANY", "USER_TO_COMPANY_CONF_FILE",
            g_setting._user2company_conf_file,
            sizeof(g_setting._user2company_conf_file), "");
    if (g_setting._user2company_conf_file[0] == 0x00) {
        conf_uninit();
        return -3;
    }

    g_setting._scenic2company_conf_file[0] = 0x00;
    read_conf_str("COMPANY", "SCENIC_TO_COMPANY_CONF_FILE",
            g_setting._scenic2company_conf_file,
            sizeof(g_setting._scenic2company_conf_file), "");
    if (g_setting._scenic2company_conf_file[0] == 0x00) {
        conf_uninit();
        return -4;
    }

    g_setting._company2company_conf_file[0] = 0x00;
    read_conf_str("COMPANY", "COMPANY_TO_COMPANY_CONF_FILE",
            g_setting._company2company_conf_file,
            sizeof(g_setting._company2company_conf_file), "");
    if (g_setting._company2company_conf_file[0] == 0x00) {
        conf_uninit();
        return -5;
    }

    g_setting._push_conf_file[0] = 0x00;
    read_conf_str("COMPANY", "PUSH_CONF_FILE", g_setting._push_conf_file,
            sizeof(g_setting._push_conf_file), "");
    if (g_setting._push_conf_file[0] == 0x00) {
        conf_uninit();
        return -6;
    }

    g_setting._company_conf_file[0] = 0x00;
    read_conf_str("COMPANY", "COMPANY_CONF_FILE", g_setting._company_conf_file,
            sizeof(g_setting._company_conf_file), "");
    if (g_setting._company_conf_file[0] == 0x00) {
        conf_uninit();
        return -7;
    }

    g_setting._company_match_queue_conf_file[0] = 0x00;
    read_conf_str("COMPANY", "COMPANY_CONF_FILE",
            g_setting._company_match_queue_conf_file,
            sizeof(g_setting._company_match_queue_conf_file), "");
    if (g_setting._company_match_queue_conf_file[0] == 0x00) {
        conf_uninit();
        return -8;
    }

    conf_uninit();

    if ( _init_service(&company_service._company, g_setting._company_conf_file) < 0 ||
         _init_service(&company_service._user2company, g_setting._user2company_conf_file) < 0 ||
         _init_service(&company_service._scenic2company, g_setting._scenic2company_conf_file) < 0 ||
         _init_service(&company_service._company2scenic, g_setting._company2scenic_conf_file) < 0 ||
         _init_service(&company_service._company2company, g_setting._company2company_conf_file) < 0 ||
         _init_service(&company_service._company_match_queue, g_setting._company_match_queue_conf_file) < 0 ||
         _init_service(&company_service._push, g_setting._push_conf_file) < 0)
    {
        return -9;
    }

    return 0;
}

int company_match_free() {
    if (company_service._company2scenic)
        sm_uninit(company_service._company2scenic);
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
