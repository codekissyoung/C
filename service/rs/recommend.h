#ifndef RS_RECOMMEND_H
#define RS_RECOMMEND_H

#include "user_graph.h"
#include "common/server_manager.h"

UserGraph *InitUserGraph(const char *fname, const char *pre_filter,
                         const char *post_filter, int weight);
int WriteRecommendToMongo(UserGraph *graph, svr_mgr_t *mongo, int suggest_num,
                          int weight_limit, int middle_user_limit);

#endif
