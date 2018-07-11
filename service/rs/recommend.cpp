#include "recommend.h"
#include "user_node.h"
#include "rs.h"
#include "mongo_handle.h"
#include "common/server_manager.h"
#include "frame/log.h"
#include <vector>
#include <string>
#include <set>
#include <fstream>

using namespace std;

struct Recommend {
  Recommend() {}
  Recommend(int num) : result(num, RecommendInfo()) {}
  svr_mgr_t *mongo_mgr;
  UserGraph *graph;
  int recommend_num;
  int weight_limit;
  int middle_limit; //中间人数量最低限制

  // 第一个是推荐结果，第二个是推荐人
  vector<RecommendInfo> result;
};

static int update_recommend(void *conn, const char *coll,
                            const char *update_key, uint64_t user_id,
                            vector<RecommendInfo> &result, int result_num,
                            Recommend *rcd) {
  char uid_str[30];
  snprintf(uid_str, sizeof(uid_str), "%" PRIu64, user_id);
  mongo_unset_key(conn, QY_MONGO_DB, coll, "uid", uid_str, update_key);
  char buf[128] = {0};
  for (int i = 0; i < result_num; i++) {
    UserNode *node = result[i].to_user;
    if (!node) {
      log_txt_err("user node null, uid:%" PRIu64 "\n", user_id);
      continue;
    }
    // 当中间人数小于最低人数限制时，不推荐该用户
    if (result[i].user_num < rcd->middle_limit)
      continue;
    snprintf(buf, sizeof(buf) - 1, "%" PRIu64 ":%" PRIu64 ":%d", node->user_id,
             result[i].from_user->user_id, result[i].user_num);
    int ret = mongo_update_add_set(conn, QY_MONGO_DB, coll, "uid", uid_str,
                                   update_key, buf);
    if (ret) {
      log_txt_err("update mongo failed\n");
    }
  }
  return 0;
}

static int update_star_recommend(void *conn, const char *coll,
                                 const char *update_key, uint64_t user_id,
                                 vector<RecommendInfo> &result, int result_num,
                                 Recommend *) {

  char uid_str[30];
  snprintf(uid_str, sizeof(uid_str), "%" PRIu64, user_id);
  mongo_unset_key(conn, QY_MONGO_DB, coll, "uid", uid_str, update_key);
  char buf[128] = {0};
  for (int i = 0; i < result_num; i++) {
    UserNode *node = result[i].to_user;
    if (!node) {
      log_txt_err("user node null, uid:%" PRIu64 "\n", user_id);
      continue;
    }
    snprintf(buf, sizeof(buf) - 1, "%" PRIu64 ":%" PRIu64,
             result[i].to_user->user_id, result[i].from_user->user_id);
    int ret = mongo_update_add_set(conn, QY_MONGO_DB, coll, "uid", uid_str,
                                   update_key, buf);
    if (ret) {
      log_txt_err("update mongo failed\n");
    }
  }
  return 0;
}

static int create_user_recommend(void *arg, UserNode *node) {
  Recommend *rcd = (Recommend *)arg;
  UserGraph *graph = rcd->graph;

  svr_group_t *mongo = sm_get_svr(rcd->mongo_mgr, node->user_id);
  void *conn = mongo->_cur_conn;

  int num = graph->GetFollowTopN(node, rcd->recommend_num, rcd->weight_limit,
                                 rcd->result);
  update_recommend(conn, QY_RECOMMEND_COLL, "rs_follow", node->user_id,
                   rcd->result, num, rcd);

  num = graph->GetFriendTopN(node, rcd->recommend_num, rcd->weight_limit,
                             rcd->result);
  update_recommend(conn, QY_RECOMMEND_COLL, "rs_friend", node->user_id,
                   rcd->result, num, rcd);

  num =
      graph->GetStars(node, rcd->recommend_num, rcd->weight_limit, rcd->result);
  update_star_recommend(conn, QY_RECOMMEND_COLL, "rs_star", node->user_id,
                        rcd->result, num, rcd);

  return 0;
}

int WriteRecommendToMongo(UserGraph *graph, svr_mgr_t *mongo, int recommend_num,
                          int weight_limit, int middle_limit) {
  Recommend rcd(recommend_num);
  rcd.mongo_mgr = mongo;
  rcd.graph = graph;
  rcd.recommend_num = recommend_num;
  rcd.weight_limit = weight_limit;
  rcd.middle_limit = middle_limit;
  graph->Traverse(create_user_recommend, &rcd);
  return 0;
}

// user_graph_file 文件格式如下:
// user_id post_num fans_num follow_num
// follow_uid
// follow_uid
// ...
//
// user_id post_num fans_num follow_num
// ...
//
//

static int get_set_list_file(const char *fname, set<uint64_t> &uids) {
  fstream fs_graph(fname, std::ios::in);
  if (!fs_graph.is_open()) {
    log_txt_err("can not open faile: %s\n", fname);
    return 0;
  }
  char line[260] = {0};
  int line_size = sizeof(line) - 1;
  while (fs_graph.getline(line, line_size)) {
    uint64_t uid = strtoll(line, NULL, 10);
    uids.insert(uid);
  }
  return uids.size();
}

UserGraph *InitUserGraph(const char *fname, const char *pre_filter_file,
                         const char *post_filter_file, int weight) {
  set<uint64_t> pre_filter;
  set<uint64_t> post_filter;
  if (pre_filter_file) {
    get_set_list_file(pre_filter_file, pre_filter);
  }
  if (post_filter_file) {
    get_set_list_file(post_filter_file, post_filter);
  }
  UserGraph *graph = new UserGraph(weight);
  if (!graph)
    return NULL;
  graph->SetPreFilter(pre_filter);
  graph->SetPreFilter(post_filter);
  fstream fs_graph(fname, std::ios::in);
  char line[260] = {0};
  int line_size = sizeof(line) - 1;
  char *end;
  while (fs_graph.getline(line, line_size)) {
    uint64_t uid = strtoll(line, &end, 10);
    int post_num = strtol(end + 1, &end, 10);
    int fans_num = strtol(end + 1, &end, 10);
    int follow_num = strtol(end + 1, &end, 10);
    uint64_t *follows = new uint64_t[follow_num];
    for (int i = 0; i < follow_num; i++) {
      fs_graph.getline(line, line_size);
      follows[i] = strtoll(line, NULL, 10);
    }
    graph->AddUser(uid, fans_num, post_num, follow_num, follows);
    fs_graph.getline(line, line_size);
  }
  fs_graph.close();
  if (graph->Create()) {
    fprintf(stderr, "user graph create failed\n");
  }
  return graph;
}
