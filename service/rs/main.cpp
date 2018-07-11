#include "recommend.h"
#include "mongo_handle.h"
#include "frame/conf.h"
#include "common/server_manager.h"

#include <stdio.h>
#include <vector>
#include <fstream>

using namespace std;

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

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("usage: ./recommend rs.ini\n");
    return 0;
  }

  const char *conf_file = argv[1];
  rs_conf_t g_conf;
  memset(&g_conf, 0, sizeof(rs_conf_t));

  int ret = conf_init(conf_file);
  if (ret != 0) {
    printf("init config file failed, <%s>", conf_file);
    return -1;
  }
  read_conf_str("RS", "MONGO_RECOMMEND_CONF_FILE",
                g_conf.mongo_recommend_conf_file,
                sizeof(g_conf.mongo_recommend_conf_file), "");
  if (g_conf.mongo_recommend_conf_file[0] == 0) {
    printf("MONGO_RECOMMEND_CONF_FILE config not exist");
    conf_uninit();
    return -2;
  }
  read_conf_str("RS", "USER_GRAPH_FILE", g_conf.user_graph_file,
                sizeof(g_conf.user_graph_file), "");
  if (g_conf.user_graph_file[0] == 0) {
    conf_uninit();
    printf("USER_GRAPH_FILE config not exist");
    return -2;
  }

  read_conf_str("RS", "PRE_FILTER_FILE", g_conf.pre_filter_file,
                sizeof(g_conf.pre_filter_file), "");

  read_conf_str("RS", "POST_FILTER_FILE", g_conf.post_filter_file,
                sizeof(g_conf.post_filter_file), "");

  read_conf_int("RS", "DEFAULT_RECOMMEND_NUM", &g_conf.recommend_num, 10);
  read_conf_int("RS", "GRAPH_WEIGHT_LIMIT", &g_conf.graph_weight_limit, 0);
  read_conf_int("RS", "USER_WEIGHT_LIMIT", &g_conf.user_weight_limit, 0);
  read_conf_int("RS", "MIDDLE_USER_LIMIT", &g_conf.middle_user_limit, 0);

  conf_uninit();
  svr_mgr_t *svr_mgr = sm_init(g_conf.mongo_recommend_conf_file,
                               mongo_create_conn, mongo_destory_conn);
  UserGraph *graph =
      InitUserGraph(g_conf.user_graph_file, g_conf.pre_filter_file,
                    g_conf.post_filter_file, g_conf.graph_weight_limit);

  WriteRecommendToMongo(graph, svr_mgr, g_conf.recommend_num,
                        g_conf.user_weight_limit, g_conf.middle_user_limit);
  delete graph;
  return 0;
}
