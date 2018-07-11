#include "user_graph.h"
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <inttypes.h>
#include <set>
#include <vector>
#include <algorithm>

#include "user_node.h"
using namespace std;

UserGraph::UserGraph() : weight_limit_(0) {}
UserGraph::UserGraph(int weight) : weight_limit_(weight) {}

UserGraph::~UserGraph() {
  for (auto &node : user_map_) {
    delete node.second;
  }
}

// return
// 1: user already exist;
int UserGraph::AddUser(uint64_t uid, int fans_num, int post_num, int follow_num,
                       uint64_t *follows) {
  if (user_map_.find(uid) != user_map_.end()) {
    return 1;
  }
  UserNode *node = new UserNode(uid, fans_num, post_num, follow_num, follows);
  if (!node) {
    fprintf(stderr, "UserNode new failed, errno:%d\n", errno);
    return -1;
  }
  user_map_[node->user_id] = node;
  return 0;
}

int UserGraph::AddUser(UserNode *node) {
  user_map_[node->user_id] = node;
  return 0;
}

int UserGraph::Create() {
  for (auto &node : user_map_) {
    node.second->ConnectFollow(this, this->weight_limit_);
  }
  for (auto &node : user_map_) {
    node.second->ConnectFriend();
  }
  return 0;
}

int UserGraph::GetFollowTopN(UserNode *user_node, int N, int weight_limit,
                             vector<RecommendInfo> &topn) {
  unordered_map<UserNode *, RecommendInfo> user_count;
  for (auto &node : user_node->follows) {
    for (auto &node2 : node->follows) {
      if (user_node != node2 && node2->weight >= weight_limit &&
          !IsPostFilter(node2->user_id) &&
          user_node->follows.find(node2) == user_node->follows.end()) {
        // user_count[node2]++;
        auto it = user_count.find(node2);
        if (it != user_count.end()) {
          it->second.user_num++;
          if (it->second.from_user->fans_num < node->fans_num) {
            it->second.from_user = node;
          }
        } else {
          user_count.insert(pair<UserNode *, RecommendInfo>(
              node2, RecommendInfo(1, node, node2)));
        }
      }
    }
  }

  vector<pair<int, RecommendInfo *>> freq(user_count.size());
  size_t index = 0;
  for (auto &item : user_count) {
    freq[index].first = item.second.user_num;
    freq[index].second = &item.second;
    index++;
  }
  int k = N < (int)freq.size() ? N : freq.size();
  nth_element(freq.begin(), freq.begin() + N, freq.end(),
              greater<pair<int, RecommendInfo *>>());
  for (int i = 0; i < k; i++) {
    // topn[i] = freq[i];
    topn[i].to_user = freq[i].second->to_user;
    topn[i].user_num = freq[i].second->user_num;
    topn[i].from_user = freq[i].second->from_user;
  }
  return k;
}

int UserGraph::GetFriendTopN(UserNode *user_node, int N, int weight_limit,
                             vector<RecommendInfo> &topn) {
  unordered_map<UserNode *, RecommendInfo> user_count;
  for (auto &node : user_node->friends) {
    for (auto &node2 : node->friends) {
      if (user_node != node2 && node2->weight >= weight_limit &&
          !IsPostFilter(node2->user_id) &&
          user_node->follows.find(node2) == user_node->follows.end()) {
        // user_count[node2]++;
        auto it = user_count.find(node2);
        if (it != user_count.end()) {
          it->second.user_num++;
          if (it->second.from_user->fans_num < node->fans_num) {
            it->second.from_user = node;
          }
        } else {
          user_count.insert(pair<UserNode *, RecommendInfo>(
              node2, RecommendInfo(1, node, node2)));
        }
      }
    }
  }

  vector<pair<int, RecommendInfo *>> freq(user_count.size());
  size_t index = 0;
  for (auto &item : user_count) {
    freq[index].first = item.second.user_num;
    freq[index].second = &item.second;
    index++;
  }
  int k = N < (int)freq.size() ? N : freq.size();
  nth_element(freq.begin(), freq.begin() + N, freq.end(),
              greater<pair<int, RecommendInfo *>>());
  for (int i = 0; i < k; i++) {
    // topn[i] = freq[i];
    topn[i].to_user = freq[i].second->to_user;
    topn[i].user_num = freq[i].second->user_num;
    topn[i].from_user = freq[i].second->from_user;
  }
  return k;
}

int UserGraph::GetStars(UserNode *node, int N, int weight_limit,
                        vector<RecommendInfo> &stars) {
  int k = 0;
  int i = 0;
  for (auto &item : node->stars) {
    for (auto &item2 : item->stars) {
      if (k < N && item2 != node && item2->weight >= weight_limit &&
          !IsPostFilter(item2->user_id) &&
          node->follows.find(item2) == node->follows.end()) {
        // 判断该用户是否已经在star推荐列表里
        for (i = 0; i < k; i++) {
          if (stars[i].to_user == item2) {
            break;
          }
        }
        if (i == k) {
          stars[k].to_user = item2;
          stars[k].from_user = item;
          k++;
        }
        break;
      }
    }
  }
  return k;
}

// node1 is follow node2 ?
bool UserGraph::IsFollow(UserNode *node1, UserNode *node2) {
  return !(node1->follows.find(node2) == node1->follows.end());
}

UserNode *UserGraph::GetUserNode(uint64_t uid) {
  auto node = user_map_.find(uid);
  return node == user_map_.end() ? NULL : node->second;
}

void UserGraph::Dump() {
  for (auto &item : user_map_) {
    printf("%" PRIu64 "\n", item.second->user_id);
  }
}

int UserGraph::Traverse(UserHandle handle, void *arg) {
  for (auto &item : user_map_) {
    if (handle(arg, item.second) == -1) {
      return -1;
    }
  }
  return user_map_.size();
}
