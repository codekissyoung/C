#include "rs/rs.h"
#include <set>
#include <string>

#include "rs/user_graph.h"
#include "rs/user_node.h"
#include "rs/recommend.h"
#include "rs/mongo_handle.h"
#include "common/common.h"
#include "common/interface.h"
#include "common/server_manager.h"
#include "frame/log.h"
#include "frame/buffer.h"
#include "frame/conf.h"

using namespace std;

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// static int g_running = 0;

struct rs_conf_t {
  char mongo_recommend_conf_file[MAX_FILE_PATH_LEN];
  char user_graph_file[MAX_FILE_PATH_LEN];
  char pre_filter_file[MAX_FILE_PATH_LEN];
  char post_filter_file[MAX_FILE_PATH_LEN];
  int recommend_num;      // 写入数据库的推荐用户数量
  int user_weight_limit;  // 用户推荐时权重最低限制
  int graph_weight_limit; // 建立图模型时权重最低限制
  int middle_user_limit;  // 推荐中间人的最低人数限制
};

static svr_mgr_t *g_svr_mongo;
static rs_conf_t g_conf;
static UserGraph *g_graph;

static int _build_failed_pack(struct io_buff *buff) {
  req_pack_t *req = reinterpret_cast<req_pack_t *>(buff->rbuff);
  rsp_pack_t *rsp = reinterpret_cast<rsp_pack_t *>(buff->wbuff);

  rsp->_header.len = sizeof(rsp_pack_t);
  rsp->_header.magic = req->_header.magic;
  rsp->_header.cmd = req->_header.cmd;
  rsp->_header.sequence = req->_header.sequence + 1;
  rsp->_header.state = -1;
  return 0;
}

static int _create_mongo_conn(char *host, short port, void **conn) {
  return mongo_create_conn(host, port, conn);
}

static int _destroy_mongo_conn(void *conn) { return mongo_destory_conn(conn); }

static int _init_mongo_service(svr_mgr_t **svr, const char *conf) {
  svr_mgr_t *tmp = sm_init(conf, _create_mongo_conn, _destroy_mongo_conn);
  if (!tmp) {
    log_txt_err("server manager init failed, conf_file[%s]", conf);
    return -1;
  }
  *svr = tmp;
  return 0;
}

static int mongo_add_set(void *conn, const char *coll, const char *update_key,
                         uint64_t user_id, uint64_t add_uid) {
  return mongo_update_add_set(conn, QY_MONGO_DB, coll, "uid",
                              std::to_string(user_id).c_str(), update_key,
                              std::to_string(add_uid).c_str());
}

// 从mongo数组中移除
// update_key : 数组名称
// user_id : 用户ID，该条记录索引key
// pull_id : 需要移除的用户ID
static int mongo_pull(void *conn, const char *coll, const char *update_key,
                      uint64_t user_id, uint64_t pull_uid) {
  return mongo_update_pull(conn, QY_MONGO_DB, coll, "uid",
                           std::to_string(user_id).c_str(), update_key,
                           std::to_string(pull_uid).c_str());
}

//**************************************************************

static int command_follow(is2rs_command_t *req) {
  svr_group_t *mongo = sm_get_svr(g_svr_mongo, req->_user_id);
  int ret = mongo_add_set(mongo->_cur_conn, QY_RELATION_COLL, "follow",
                          req->_user_id, req->_other_id);
  ret = mongo_pull(mongo->_cur_conn, QY_RELATION_COLL, "unfollow",
                   req->_user_id, req->_other_id);
  return ret;
}

static int command_unfollow(is2rs_command_t *req) {
  svr_group_t *mongo = sm_get_svr(g_svr_mongo, req->_user_id);
  int ret = mongo_add_set(mongo->_cur_conn, QY_RELATION_COLL, "unfollow",
                          req->_user_id, req->_other_id);
  ret = mongo_pull(mongo->_cur_conn, QY_RELATION_COLL, "follow", req->_user_id,
                   req->_other_id);
  return ret;
}

static int command_disinterest(is2rs_command_t *req) {
  svr_group_t *mongo = sm_get_svr(g_svr_mongo, req->_user_id);
  return mongo_add_set(mongo->_cur_conn, QY_RELATION_COLL, "disinterest",
                       req->_user_id, req->_other_id);
}

// 运行用户推荐，并将结果写入mongo
static int command_run_all(is2rs_command_t *) {
  log_txt_info("recommend_num:%d, user_weight_limit:%d", g_conf.recommend_num,
               g_conf.user_weight_limit);
  WriteRecommendToMongo(g_graph, g_svr_mongo, g_conf.recommend_num,
                        g_conf.user_weight_limit, g_conf.middle_user_limit);
  return 0;
}

static int _proc_command(struct io_buff *buff) {
  is2rs_command_t *req = reinterpret_cast<is2rs_command_t *>(buff->rbuff);
  rs2is_command_t *rsp = reinterpret_cast<rs2is_command_t *>(buff->wbuff);
  int ret = 0;
  switch (req->command_type) {
  case RS_COMMAND_FOLLOW:
    ret = command_follow(req);
    break;
  case RS_COMMAND_UNFOLLOW:
    ret = command_unfollow(req);
    break;
  case RS_COMMAND_DISINTEREST:
    ret = command_disinterest(req);
    break;
  case RS_COMMAND_RUN_ALL:
    ret = command_run_all(req);
    break;
  }
  rsp->_header.len = sizeof(rs2is_command_t);
  rsp->_header.magic = req->_header.magic;
  rsp->_header.cmd = req->_header.cmd;
  rsp->_header.sequence = req->_header.sequence + 1;
  rsp->_header.state = 0;
  return ret;
}

int rs_init(const char *conf_file) {
  memset(&g_conf, 0, sizeof(g_conf));
  int ret = conf_init(conf_file);
  if (ret != 0) {
    log_txt_err("init config file failed, <%s>", conf_file);
    return -1;
  }
  g_conf.mongo_recommend_conf_file[0] = 0;
  read_conf_str("RS", "MONGO_RECOMMEND_CONF_FILE",
                g_conf.mongo_recommend_conf_file,
                sizeof(g_conf.mongo_recommend_conf_file), "");
  if (g_conf.mongo_recommend_conf_file[0] == 0) {
    log_txt_err("MONGO_RECOMMEND_CONF_FILE config not exist");
    conf_uninit();
    return -2;
  }
  g_conf.user_graph_file[0] = 0;
  read_conf_str("RS", "USER_GRAPH_FILE", g_conf.user_graph_file,
                sizeof(g_conf.user_graph_file), "");
  if (g_conf.user_graph_file[0] == 0) {
    conf_uninit();
    log_txt_err("USER_GRAPH_FILE config not exist");
    return -2;
  }

  g_conf.pre_filter_file[0] = 0;
  read_conf_str("RS", "PRE_FILTER_FILE", g_conf.pre_filter_file,
                sizeof(g_conf.pre_filter_file), "");

  g_conf.post_filter_file[0] = 0;
  read_conf_str("RS", "POST_FILTER_FILE", g_conf.post_filter_file,
                sizeof(g_conf.post_filter_file), "");


  read_conf_int("RS", "DEFAULT_RECOMMEND_NUM", &g_conf.recommend_num, 10);
  read_conf_int("RS", "GRAPH_WEIGHT_LIMIT", &g_conf.graph_weight_limit, 0);
  read_conf_int("RS", "USER_WEIGHT_LIMIT", &g_conf.user_weight_limit, 0);
  read_conf_int("RS", "MIDDLE_USER_LIMIT", &g_conf.middle_user_limit, 0);

  conf_uninit();
  ret = _init_mongo_service(&g_svr_mongo, g_conf.mongo_recommend_conf_file);
  if (ret != 0) {
    log_txt_err("init mongo service failed");
    return -3;
  }
  log_txt_info("recommend_num:%d", g_conf.recommend_num);
  log_txt_info("graph_weight_limit:%d", g_conf.graph_weight_limit);
  log_txt_info("user_weight_limit:%d", g_conf.user_weight_limit);
  log_txt_info("middle_user_limit:%d", g_conf.middle_user_limit);
  log_txt_info("pre_filter_file:%s", g_conf.pre_filter_file);
  log_txt_info("post_filter_file:%s", g_conf.post_filter_file);
  g_graph = InitUserGraph(g_conf.user_graph_file, g_conf.pre_filter_file,
                          g_conf.post_filter_file, g_conf.graph_weight_limit);
  if (!g_graph) {
    log_txt_err("create user graph failed");
    return -4;
  }
  return 0;
}

int rs_proc(struct io_buff *buff) {
  req_pack_t *req = reinterpret_cast<req_pack_t *>(buff->rbuff);
  int ret = 0;
  switch (req->_header.cmd) {
  case CMD_RS_COMMAND:
    ret = _proc_command(buff);
  default:
    break;
  }
  if (ret < 0) {
    _build_failed_pack(buff);
  }
  return 0;
}

int rs_uninit() {
  if (g_svr_mongo) {
    sm_uninit(g_svr_mongo);
  }
  return 0;
}
