#include "user_node.h"
#include "user_graph.h"
#include <inttypes.h>
#include <algorithm>
using namespace std;

#define DEFAULT_STAR_NUM 20

int UserNode::ConnectFollow(UserGraph *graph, int weigh_limit) {
  if (!this->follow_uids_)
    return 0;

  vector<pair<int, UserNode *>> users;
  for (int i = 0; i < follow_num_; i++) {
    if (graph->IsPreFilter(follow_uids_[i])) {
      // 节点在前置过滤条件下，将不再有人关注TA
      continue;
    }
    UserNode *node = graph->GetUserNode(follow_uids_[i]);
    if (node && node->weight >= weigh_limit) {
      this->follows.insert(node);
      users.push_back(pair<int, UserNode *>(node->fans_num, node));
    }
  }
  // 计算关注的人中权重最高的前N个人
  int star_num = DEFAULT_STAR_NUM;
  int n = star_num < (int)users.size() ? star_num : users.size();
  partial_sort(users.begin(), users.begin() + n, users.end(),
               greater<pair<int, UserNode *>>());
  for (int i = 0; i < n; i++) {
    this->stars.push_back(users[i].second);
  }
  delete[] follow_uids_;
  follow_uids_ = NULL;
  follow_num_ = 0;
  return 0;
}

// after user node connect, will connect friend
int UserNode::ConnectFriend() {
  for (auto &item : follows) {
    if (item->follows.find(this) != item->follows.end()) {
      this->friends.insert(item);
    }
  }
  return 0;
}
