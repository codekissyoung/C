#include <stdint.h>
#include <set>
#include <vector>

class UserGraph;

class UserNode {
  public:
    UserNode() {}
    UserNode(uint64_t uid, int fans_num, int post_num,
        int follow_num, uint64_t* follow_uids) :
      user_id(uid),
      fans_num(fans_num),
      post_num(post_num),
      follow_uids_(follow_uids),
      follow_num_(follow_num) {
        if (post_num == 0) {
          weight = 0;
        } else {
          weight = fans_num;
        }
      }

    uint64_t user_id;
    int fans_num;
    int post_num;
    int weight;
    std::set<UserNode*> follows;
    std::set<UserNode*> friends;
    std::vector<UserNode*> stars;

    // 
    int ConnectFollow(UserGraph* graph, int weight);
    int ConnectFriend();

  private:
    uint64_t* follow_uids_;
    int follow_num_;
};
