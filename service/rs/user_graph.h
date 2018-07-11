#ifndef QY_USER_GRAPH_H_
#define QY_USER_GRAPH_H_

#include <stdint.h>
#include <unordered_map>
#include <vector>
#include <set>

class UserNode;
typedef int (*UserHandle)(void *input, UserNode *node);

struct RecommendInfo {
  RecommendInfo() : user_num(0) {}
  RecommendInfo(int num, UserNode *from, UserNode *to)
      : user_num(num), from_user(from), to_user(to) {}
  int user_num;        //推荐用户数量
  UserNode *from_user; // 推荐人
  UserNode *to_user;   // 推荐结果
};

class UserGraph {
public:
  UserGraph();
  UserGraph(int weight);
  ~UserGraph();

  // Just use for init usergraph
  int AddUser(uint64_t uid, int fans_num, int post_num, int follows_num,
              uint64_t *follows);
  int AddUser(UserNode *user_node);
  // after AddUser, must call Create
  int Create();

  UserNode *GetUserNode(uint64_t uid);

  // call when a new user register
  // UserGraph new a user
  // int NewUser(uint64_t uid, uint64_t* follows, int follows_num);

  int GetRecommend(UserNode *user_node, int N, int weight_limit,
                   std::vector<RecommendInfo> &topn);

  int GetFollowTopN(UserNode *user_node, int N, int weight_limit,
                    std::vector<RecommendInfo> &topn);

  int GetFriendTopN(UserNode *user_node, int N, int weight_limit,
                    std::vector<RecommendInfo> &topn);

  // stars: 第一个为推荐结果，第二个为推荐用户
  int GetStars(UserNode *user_node, int N, int weight_limit,
               std::vector<RecommendInfo> &stars);

  // is node1 follow node2 ?
  bool IsFollow(UserNode *node1, UserNode *node2);
  // int Follow(UserNode* user_node, uint64_t uid);
  // int UnFollow(UserNode* user_node, uint64_t uid);

  int Traverse(UserHandle handle, void *arg);

  inline void SetPreFilter(std::set<uint64_t> &pre_filter) {
    pre_filter_ = pre_filter;
  }

  inline void SetPostFilter(std::set<uint64_t> &post_filter) {
    post_filter_ = post_filter;
  }

  inline bool IsPreFilter(uint64_t uid) {
    return pre_filter_.find(uid) != pre_filter_.end();
  }

  inline bool IsPostFilter(uint64_t uid) {
    return post_filter_.find(uid) != post_filter_.end();
  }

  void Dump();

private:
  std::unordered_map<uint64_t, UserNode *> user_map_;
  std::set<uint64_t> pre_filter_;
  std::set<uint64_t> post_filter_;
  int weight_limit_;
};

#endif // QY_USER_GRAPH_H_
