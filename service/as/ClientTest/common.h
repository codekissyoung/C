#ifndef __COMMON_H_
#define __COMMON_H_

// 重新定义uint64_t
//typedef unsigned long long int	uint64_t;

/* 一个用户最多follow其他用户的数量 */
#define MAX_FOLLOWER_NUM   1000

/* 一次请求，最多返回的post数量 */
#define MAX_RET_POST_NUM   200

/* 一次请求，最多返回的atme文章或评论数量 */
#define MAX_RET_ATME_NUM   200

/* 一次请求，最多返回的comment数量 */
#define MAX_RET_COMMENT_NUM   200

/* "查看对话"中评论链的最大长度 */
#define MAX_CMT_NUM_PER_THREAD 20

/* 后台服务host最大长度 */
#define MAX_SVR_HOST_LEN   50

/* redis key前缀的最大长度 */
#define MAX_RDS_PRFX_LEN   20

/* 一组服务中，最多节点数（一组服务中各节点相互容灾） */
#define MAX_NODE_NUM_PER_GROUP 3

/* 一条post最多属于多少个user. 注意, 在青驿的设计框架中 */
/* 用户个人和景点共享一套ID体系. 我们在景点页查看某景点 */
/* 关联的所有post, 就和在个人页查看某个用户发表的所有   */
/* post 一样.                                           */
/* 一篇post中可以关联多个景点.                          */

#define MAX_REF_CNT_PER_POST 50

/* 一条post最多tag数量 */
#define MAX_TAG_CNT_PER_POST 6

/* as2bs set post的数据包中, 所有可能的user_id 和 tag   */
/* 组合中, 遍历所有 tags 的连接长度.                    */
/* 52 代表 all: travellog: roadmap: discussion:         */
/* purchase: advertise: 6种组合的总长度.                */

#define MAX_USER_TAG_LEN (MAX_REF_CNT_PER_POST+1)*52

/* 用于关联到景点的文本最大长度, 默认3万汉字长度.      */
#define MAX_TXT_LEN_USED_TO_REF 2*3*10*1024

/* 文件路径最大长度 */
#define MAX_FILE_PATH_LEN 256

/* 景点名最大长度 */
#define MAX_NAME_LEN 128

/* 一篇文章中at其他人的最大数量 */
#define MAX_AT_USER_NUM 50

/* 输入时请求自动建议结果的输入串最大长度 */
#define MAX_SUGGESTION_QUERY_LEN 30
/* 一次输入中自动建议结果的最大数量 */
#define MAX_SUGGESTION_NUM 20
/* 一次输入中自动建议结果的(景点)汉字最大长度 */
#define MAX_SUGGESTION_CHN_LEN 20*3
/* 一次输入中自动建议结果的(景点)英文最大程度 */
#define MAX_SUGGESTION_EN_LEN 100

/* 用户推荐数量*/
#define MAX_RECOMMEND_NUM 50

/* 一次请求，最多返回的结伴数量 */
#define MAX_RET_COMPANY_NUM   200

/* 一个结伴需求最多关联的景点数  */
#define MAX_COMPANY_SCENIC_NUM   8

/* 用户收到的 post 中 at 提醒 */
#define PFX_ATME    "atme"

/* 用户收到的 comment 中 at 提醒 */
#define PFX_ATME_CMT    "atme_cmt"

/* 收藏前缀 */
#define PFX_ENSHRINE    "enshrine"

/**** comment 相关常量 ****/

/* post2comment前缀 */
#define PFX_POST2COMMENT    "post2comment"

/* comment2post前缀 */
#define PFX_COMMENT2POST    "comment2post"

/* comment2comment前缀 */
#define PFX_COMMENT2COMMENT    "comment2comment"

/* 用户收到的评论 */
#define PFX_USER2COMMENT    "user2comment"

/* 用户发出的评论 */
#define PFX_USER_COMMENT    "user_comment"

/* 每篇post对应的user_id, 含作者和景点ID. */
#define PFX_POST2USER	"post2user"

/* 每篇post对应的at用户 */
#define PFX_POST2AT	"post2at"

/* 2016.07.05: 旅游产品页的评论索引      */
#define PFX_PRODUCTCMT2CMT    "productcmt2cmt"
#define PFX_PRODUCT2COMMENT    "product2comment"
#define PFX_USER2PRODUCTCMT    "user2productcmt"

/* 2016.07.13: 结伴服务相关索引      */
#define PFX_COMPANY2SCENIC    "company2scenic"
#define PFX_USER2COMPANY    "user2company"
#define PFX_SCENIC2COMPANY    "scenic2company"
#define PFX_COMPANY2COMPANY    "company2company"
#define PFX_COMPANY          "company:"

/* 2016.08.16: 打赏服务相关索引     */

/* 请求文章一次, 最多返回200个打赏者的ID. */
#define MAX_PAIDUSER_CNT_PER_POST 200

#define PFX_POST2PAIDUSER    "post2paiduser"


/* 推送 */
#define PFX_PUSH    "push"

#endif
