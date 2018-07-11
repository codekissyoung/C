#ifndef __INTERFACE_H
#define __INTERFACE_H

#include <stdint.h>
#include "protocol.h"
#include "common.h"

/* 2015.09.01: Post 和 Comment 功能.  */

#define CMD_GET_POST 0x01
#define CMD_SET_POST 0x02
#define CMD_GET_POST_CNT 0x03
#define CMD_GET_POST_ATME 0x04
#define CMD_SET_ENSHRINE 0x05
#define CMD_GET_ENSHRINE 0x06
#define CMD_DEL_POST 0x07
#define CMD_GET_ENSHRINE_CNT 0x08
#define CMD_GET_POST_ATME_CNT 0x09
#define CMD_GET_POST_BY_PAGE 0x0A
#define CMD_GET_SCENIC_USER_BY_POST 0x0B
#define CMD_POST_SET_SCENIC 0x0C

#define SET_POST_USERID_SET 0x01
#define SET_POST_USERID_REMOVE 0x02

#define SET_ENSHRINE_TYPE_SET 0x00
#define SET_ENSHRINE_TYPE_UNSET 0x01

#define CMD_GET_COMMENT_BY_POST 0x11
#define CMD_GET_COMMENT_BY_COMMENT 0x12
#define CMD_SET_COMMENT 0x13
#define CMD_GET_COMMENT_CNT_BY_POST 0x14
#define CMD_GET_COMMENT_CNT_BY_COMMENT 0x15
#define CMD_GET_COMMENT_TOME 0x16
#define CMD_SUGGESTION_SCENIC 0x17
#define CMD_GET_COMMENT_TOME_CNT 0x18
#define CMD_SUGGESTION_USER 0x19
#define CMD_GET_COMMENT_ATME 0x1A
#define CMD_GET_COMMENT_ATME_CNT 0x1B
/*
#define CMD_GET_COMMENT_BY_USER 0x1C
#define CMD_GET_COMMENT_BY_USER_CNT 0x1D
*/
#define CMD_DEL_COMMENT 0x1E                    /* 删除该评论所有内容     */
#define CMD_DEL_POST_COMMENT 0x1F               /* 仅删除文章下该评论     */

/* 2015.12.01: 社交推荐功能.   */
#define CMD_GET_RECOMMEND 0x20
#define CMD_RS_COMMAND 0x21

#define RS_COMMAND_FOLLOW 0x01
#define RS_COMMAND_UNFOLLOW 0x02
#define RS_COMMAND_DISINTEREST 0x03
#define RS_COMMAND_TASK 0x04
#define RS_COMMAND_BLACK 0x05
#define RS_COMMAND_RUN_ALL 0x06
#define RS_COMMAND_RUN_ONE 0x07

#define RS_REASON_WEIBO 0x01                    /* 微博好友   */
#define RS_REASON_QQ 0x02                       /* QQ好友     */
#define RS_REASON_WX 0x03                       /* 微信好友   */
#define RS_REASON_CONTACT 0x04                  /* 手机通讯录 */
#define RS_REASON_SCENIC 0x05                   /* 旅行地相关 */
#define RS_REASON_NEARBY 0x06                   /* 附近的人   */

/* 2016.07.05: 旅游产品页的评论功能   */
#define CMD_GET_PRODUCT_COMMENT_BY_PRODUCT 0x22
#define CMD_GET_PRODUCT_COMMENT_BY_COMMENT 0x23
#define CMD_SET_PRODUCT_COMMENT 0x24
#define CMD_GET_COMMENT_CNT_BY_PRODUCT 0x25
#define CMD_GET_COMMENT_CNT_BY_PRODUCT_COMMENT 0x26
#define CMD_GET_PRODUCT_COMMENT_TOME 0x27
#define CMD_GET_PRODUCT_COMMENT_TOME_CNT 0x28
#define CMD_DEL_RPODUCT_COMMENT 0x29

/* 2016.08.20: 结伴功能.              */
#define CMD_GET_COMPANY 0x2A
#define CMD_GET_COMPANY_CNT 0x2B
#define CMD_ABORT_COMPANY 0x2C
#define CMD_NEW_COMPANY 0x2D
#define CMD_GET_COMPANY_TIMELINE 0x30
#define CMD_COMPANY_MANUAL_MATCH 0x31
#define CMD_COMPANY_MANUAL_DETACH 0x32
#define CMD_COMPANY_MANUAL_ATTACH_SCENICS 0x33

#define COMPANY_SET_FLAG_ABORT 0x01
#define COMPANY_SET_FLAG_DELETED 0x02
#define COMPANY_SET_FLAG_EXPIRED 0x03

#define COMPANY_REQ_TYPE_USER 0x01            /* 某用户发布的结伴需求   */
#define COMPANY_REQ_TYPE_SCENIC 0x02          /* 旅行地下关联的结伴需求 */
#define COMPANY_REQ_TYPE_COMPANY 0x03         /* 获取某个结伴的匹配结果 */
#define COMPANY_REQ_TYPE_RESULT 0x04          /* 获取用户的结伴匹配结果 */

#define COMPANY_MATCH_TYPE_NEW 0x01           /* 新建结伴时匹配         */
#define COMPANY_MATCH_TYPE_MANUAL 0x02        /* 添加旅行地后匹配       */

/* 2016.08.16: 文章下的打赏者名单.    */
#define CMD_SET_PAIDUSER 0x2E
#define CMD_GET_PAIDUSER 0x2F

/* 2016.10.26: 问答功能.              */
#define CMD_SET_QUESTION 0x34
#define CMD_SET_ANSWER 0x35
#define CMD_DEL_QA 0x36
#define CMD_ATTACH_DETACH_SCENIC 0x37
#define CMD_GET_QA_TIMELINE 0x38
#define CMD_GET_QA 0x39
#define CMD_GET_QA_CNT 0x3A
#define CMD_LAUNCH_INVITE 0X3B
#define CMD_GET_QA_INVITEME 0x3C
#define CMD_ADD_FOLLOW_Q 0X3D
#define CMD_DEL_FOLLOW_Q 0X3E
#define CMD_QUERY_FOLLOW_Q 0X3F

#define QA_REQ_TYPE_USER 0x01                 /* 根据指定的键请求问题或答案  */
#define QA_REQ_TYPE_SCENIC 0x02
#define QA_REQ_TYPE_QUESTION 0x03
#define QA_REQ_TYPE_ANSWERME 0x04
#define QA_REQ_TYPE_INVITEME 0x05

#define QA_REQ_TYPE_A_UID_FOR_Q 0x06         /* 根据指定的问题, 请求用户清单. */
#define QA_REQ_TYPE_AWAITING_UID_FOR_Q 0x07
#define QA_REQ_TYPE_FOLLOW_FOR_Q 0x08

/* 所有 service的 magic code 类型定义 */

#define MAGIC_CODE_GET_POST 0x13
#define MAGIC_CODE_SET_POST 0x13
#define MAGIC_CODE_DEL_POST 0x13
#define MAGIC_CODE_GET_POST_CNT 0x13
#define MAGIC_CODE_GET_POST_ATME 0x13
#define MAGIC_CODE_GET_POST_ATME_CNT 0x13
#define MAGIC_CODE_GET_SET_ENSHRINE 0x13
#define MAGIC_CODE_GET_GET_ENSHRINE 0x13
#define MAGIC_CODE_GET_ENSHRINE_CNT 0x13
#define MAGIC_CODE_PAIDUSER 0x13
#define MAGIC_CODE_GET_COMMENT 0x24
#define MAGIC_CODE_SET_COMMENT 0x24
#define MAGIC_CODE_SUGGESTION 0x25
#define MAGIC_CODE_RECOMMEND 0x26
#define MAGIC_CODE_COMPANY 0x27
#define MAGIC_CODE_QA 0x28

/* 所有 service的 push 类型定义 */

#define PUSH_SERVICE_TYPE_TIMELINE 0x01
#define PUSH_SERVICE_TYPE_ATME 0x02
#define PUSH_SERVICE_TYPE_NEWFANS 0x03
#define PUSH_SERVICE_TYPE_NEWCOMMENT 0x04
#define PUSH_SERVICE_TYPE_ATME_CMT 0x05
#define PUSH_SERVICE_TYPE_LETTER 0x06
#define PUSH_SERVICE_TYPE_NEW_PRODUCT_COMMENT 0x07
#define PUSH_SERVICE_TYPE_NEW_COMPANY 0x08
#define PUSH_SERVICE_TYPE_INVITEME 0x09
#define PUSH_SERVICE_TYPE_ANSWERME 0x0A

#pragma pack(1)

typedef struct {
    pack_header _header;
    char _buff[0];
} req_pack_t;

typedef struct {
    pack_header _header;
    char _buff[0];
} rsp_pack_t;

typedef struct {
    pack_header _header;
    char _tag[MAX_RDS_PRFX_LEN];
    int _user_num; /* 请求中user id数量 */
    uint64_t _user_ids[MAX_FOLLOWER_NUM]; /* 该变量需要保持在结构体最后，数组是实际请求中不会填满 */
} is2as_get_post_cnt_t;

typedef struct {
    pack_header _header;
    int _post_num;
} as2is_get_post_cnt_t;

typedef struct {
    pack_header _header;
    int _start_idx;
    int _req_num;
    char _tag[MAX_RDS_PRFX_LEN];
    int _user_num;                        /* 请求中user id数量.    */
    int _user_types[MAX_FOLLOWER_NUM];    /* 类型. 用户:1, 景点:2. */
    uint64_t _user_ids[MAX_FOLLOWER_NUM]; /* 请求时候的ID.         */
} is2as_get_post_t;

typedef struct {
    pack_header _header;
    int _post_num;                         /* 返回包中，post数量.   */
    int _user_types[MAX_RET_POST_NUM] ;    /* 类型. 用户:1, 景点:2. */
    uint64_t _user_ids[MAX_RET_POST_NUM] ; /* 请求时候的ID, 返回时用来表明某篇文章的归属. */
    uint64_t _posts[MAX_RET_POST_NUM];     /* 返回的文章的ID.       */
} as2is_get_post_t;

typedef struct {
    pack_header _header;
    uint64_t _post_id;
    short _tag_num;
    char _tags[MAX_TAG_CNT_PER_POST][MAX_RDS_PRFX_LEN]; /* post的标签 */
} is2as_del_post_t;

typedef struct {
    pack_header _header;
    char _data[0];
} as2is_del_post_t;

typedef struct {
    pack_header _header;
    uint64_t _post_id;
    uint64_t _ref_pid;
    uint64_t _user_id;
    short _tag_num;
    char _tags[MAX_TAG_CNT_PER_POST][MAX_RDS_PRFX_LEN]; /* post的标签 */
    short _at_user_num; /* 文章at其他人的数量 */
    uint64_t _at_users[MAX_AT_USER_NUM];
    int _extract_scenic_option; // 是否匹配景点，1匹配 0不匹配
    char _ref_text[MAX_TXT_LEN_USED_TO_REF]; /* 该变量需要保持在结构体最后，数组是实际请求中不会填满 */
} is2as_set_post_t;

typedef struct {
    pack_header _header;
    char _data[0];
} as2is_set_post_t;

typedef struct {
    pack_header _header;
    uint64_t _post_id;
    uint64_t _user_id;
    short _tag_num;
    char _tags[MAX_TAG_CNT_PER_POST][MAX_RDS_PRFX_LEN]; /* post的标签 */
    int _option; // 0 关联旅行地  1 取关旅行地
} is2as_post_set_scenic_t;

typedef struct {
    pack_header _header;
    char _data[0];
} as2is_post_set_scenic_t;

typedef struct {
    pack_header _header;
    char _tag[MAX_RDS_PRFX_LEN];
    int _user_num; /* 请求中user id数量 */
    uint64_t _user_ids[MAX_FOLLOWER_NUM]; /* 该变量需要保持在结构体最后，数组是实际请求中不会填满 */
} as2bs_get_post_cnt_t;

typedef struct {
    pack_header _header;
    int _post_num; /* post数量 */
} bs2as_get_post_cnt_t;

typedef struct {
    pack_header _header;
    int _start_idx;
    int _req_num;
    char _tag[MAX_RDS_PRFX_LEN];
    int _user_num; /* 请求中user id数量 */
    int _user_types[MAX_FOLLOWER_NUM];    /* 类型. 用户:1, 景点:2. */
    uint64_t _user_ids[MAX_FOLLOWER_NUM]; /* 该变量需要保持在结构体最后，数组是实际请求中不会填满 */
} as2bs_get_post_t;

typedef struct {
    pack_header _header;
    int _post_num; /* 返回包中，post数量 */
    int _user_types[MAX_RET_POST_NUM] ;    /* 类型. 用户:1, 景点:2. */
    uint64_t _user_ids[MAX_RET_POST_NUM] ; /* 请求时候的ID, 返回时用来表明某篇文章的归属. */
    uint64_t _posts[MAX_RET_POST_NUM]; /* 该变量需要保持在结构体最后，数组是实际请求中不会填满 */
} bs2as_get_post_t;

typedef struct {
    pack_header _header;

	/* "添加" tag_user 和 post 的关系, 还是"脱钩" */
	int _set_post_user_type ;

	uint64_t _post_id;

	/* user_id 和 tag 组合对的数量 */
    int _pair_cnt ;
    //uint64_t _user_ids[MAX_REF_CNT_PER_POST+1];
    uint64_t _user_ids[(MAX_REF_CNT_PER_POST+1)*MAX_TAG_CNT_PER_POST];

	/* 由于用于保存 tag 的_data[]数组是一维数组, 所以_tags[]中保存了 */
	/* 每一个tag元素在_data[] 中距离起始地址的总偏移量.              */
    /* 假设每篇文章中允许提及的景点数目为 MAX_REF_CNT_PER_POST, 加上 */
    /* 作者本人的user_id, 共对应 MAX_REF_CNT_PER_POST+1 个ID.        */

    short _tags[(MAX_REF_CNT_PER_POST+1)*MAX_TAG_CNT_PER_POST];

	/* 每篇 post 的tag, 以字符串的形式保存在 _data 中, 以'\0'分隔.   */
	char _data[MAX_USER_TAG_LEN];
} as2bs_set_post_t;

typedef struct {
    pack_header _header;
    char _data[0]; // 返回包中，只需要包头，不需要包体
} bs2as_set_post_t;

typedef struct {
    pack_header _header;
    uint64_t _user_id;
    int _start_idx;
    int _req_num;
} is2as_get_post_atme_t; // 获取@我的文章列表

typedef struct {
    pack_header _header;
    int _post_num;
    uint64_t _posts[MAX_RET_POST_NUM]; /* 该变量需要保持在结构体最后，数组是实际请求中不会填满 */
} as2is_get_post_atme_t;

typedef struct {
    pack_header _header;
    uint64_t _user_id;
    int _obj_type;      // 0:post; 1:问题; 2:答案.
    uint64_t _obj_id;
    int set_or_unset ;  // SET_ENSHRINE_TYPE_SET: 0x00 ; SET_ENSHRINE_TYPE_UNSET: 0x01
} is2as_set_enshrine_t; // 收藏接口

typedef struct {
    pack_header _header;
    char _data[0];
} as2is_set_enshrine_t;

typedef struct {
    pack_header _header;
    uint64_t _user_id;
    int _start_idx;
    int _req_num;
    int _req_type;      // 0: 老接口, 只能拉取posts; 1: 新接口, 拉取所有收藏对象.

} is2as_get_enshrine_t; // 获取收藏列表

typedef struct {
    pack_header _header;
    int _obj_num;
    int _obj_types[MAX_RET_POST_NUM]; // 0:post; 1:问题; 2:答案.
    uint64_t _obj_ids[MAX_RET_POST_NUM];
} as2is_get_enshrine_t;

typedef struct {
    pack_header _header;
    uint64_t _user_id;
} is2as_get_as_item_cnt_t;

typedef struct {
    pack_header _header;
    int _item_num;
} as2is_get_as_item_cnt_t;

/* CS 相关协议           */

typedef struct {
    pack_header _header;
    /* _obj_id 是 post_id, comment_id, user_id, product_id中任一种. */
    uint64_t _obj_id;
} is2cs_get_cnt_t;

typedef struct {
    pack_header _header;
    int _comment_num;
} cs2is_get_cnt_t;

typedef struct {
    pack_header _header;
    /* _obj_id 是 post_id, comment_id, user_id, product_id中任一种. */
    uint64_t _obj_id;
    int _start_idx;
    int _req_num;
} is2cs_get_comment_t;

typedef struct {
    pack_header _header;
    /* 返回包中，comment数量 */
    int _comment_num;
    uint64_t _comments[MAX_RET_COMMENT_NUM]; /* 该变量需要保持在结构体最后，数组是实际请求中不会填满 */
} cs2is_get_comment_t;

typedef is2cs_get_comment_t is2cs_get_thread_t;
typedef cs2is_get_comment_t cs2is_get_thread_t;

typedef struct {
    pack_header _header;
    uint64_t _user_id;
    uint64_t _comment_id;
    // uint64_t _root_post_id;
	uint64_t _post_id ;
	uint64_t _post_user_id ;

	/*     如果 _post_id 并非 root 型, 则要通过该 post 的 _post_equal_comment_id */
	/* 来查找 comment2post 索引表, 获得 _upward_post_id.                           */

	uint64_t _post_equal_comment_id ;

    uint64_t _root_user_id;
    uint64_t _parent_comment_id;
    uint64_t _parent_user_id;
    uint64_t _as_post_id;
    short _at_user_num;
    uint64_t _at_users[MAX_AT_USER_NUM];
} is2cs_set_comment_t;

/* 2016.07.05: 在各类型的产品页下发布(回复)评论. */
typedef struct {
    pack_header _header;
    uint64_t _user_id;
    uint64_t _product_cmt_id;
	uint64_t _product_id ;
	uint64_t _product_user_id ;

    uint64_t _parent_comment_id;
    uint64_t _parent_user_id;
} is2cs_set_product_cmt_t;

typedef struct {
    pack_header _header;
    char _data[0]; // 返回包中，只需要包头，不需要包体
} cs2is_set_comment_t;

typedef struct {
    pack_header _header;
    uint64_t _user_id;
    uint64_t _comment_id;
    uint64_t _parent_user_id;
    uint64_t _post_user_id;
    uint64_t _root_user_id;
} is2cs_del_comment_t;

// cs2is_del_comment_t 使用 rsp_pack_t

typedef struct {
    pack_header _header;
    uint64_t _user_id;
    uint64_t _post_id;
    uint64_t _comment_id;
} is2cs_del_post_comment_t;

// cs2is_del_post_comment_t 使用 rsp_pack_t

/* 2016.07.05: 在各类型的产品页下删除评论. */

typedef struct {
    pack_header _header;
    uint64_t _user_id;
    uint64_t _product_cmt_id;
    uint64_t _parent_user_id;
    uint64_t _product_id;
    uint64_t _product_user_id;
} is2cs_del_product_cmt_t;

// cs2is_del_product_cmt_t 使用 rsp_pack_t

typedef struct {
    pack_header _header;
    uint64_t _user_id;
    int _start_idx;
    int _req_num;
} is2cs_get_comment_tome_t; // 获取我收到的评论

typedef struct {
    pack_header _header;
    int _comment_num; /* 返回包中，comment数量 */
    uint64_t _comments[MAX_RET_COMMENT_NUM]; /* 该变量需要保持在结构体最后，数组是实际请求中不会填满 */
} cs2is_get_comment_tome_t;

/*
typedef struct {
    pack_header _header;
    uint64_t _user_id;
    int _start_idx;
    int _req_num;
} is2cs_get_comment_by_user_t;

typedef struct {
    pack_header _header;
    int _comment_num;
    uint64_t _comments[MAX_RET_COMMENT_NUM];
} cs2is_get_comment_by_user_t;
*/
typedef struct {
    pack_header _header;
    char _query[MAX_SUGGESTION_QUERY_LEN] ;
} is2ss_query_t;

typedef struct {
    pack_header _header;
    int _suggestion_num;
    uint64_t _id[MAX_SUGGESTION_NUM];
    char _chn_name[MAX_SUGGESTION_NUM][MAX_SUGGESTION_CHN_LEN];
    char _en_name[MAX_SUGGESTION_NUM][MAX_SUGGESTION_EN_LEN];
} ss2is_suggestion_t;

typedef struct {
    pack_header _header;
    uint64_t _user_id;
    int _recommend_type; // 0表示推荐全部类型
} is2rs_recommend_t;

typedef struct {
    pack_header _header;
    uint64_t _user_id;
    uint64_t _other_id;
    int command_type; //命令类型
} is2rs_command_t;

typedef struct {
    pack_header _header;
    char _data[0];
} rs2is_command_t;

typedef struct {
	pack_header _header;
    uint64_t _user_id;
    uint64_t _company_id;
    uint64_t _start_date;
    uint64_t _end_date;
    int _people_num;
    int _low_quality;
    int _scenics_num;
    uint64_t _scenics[MAX_COMPANY_SCENIC_NUM];
} is2company_new_t;

typedef struct {
	pack_header _header;
} company2is_new_t;

typedef struct {
	pack_header _header;
    uint64_t _user_id;
    uint64_t _company_id;
    int set_flag;          // 1:撤回; 2:删除; 3:过期;
    int _scenics_num;
    uint64_t _scenics[MAX_COMPANY_SCENIC_NUM];
} is2company_set_t;

typedef struct {
	pack_header _header;
} company2is_set_t;

typedef struct {
	pack_header _header;
    uint64_t _obj_id; // 用户，景点, 或结伴id
    int _req_type;    // COMPANY_REQ_TYPE_
    int _start_idx;
    int _req_num;
} is2company_get_t;

typedef struct {
	pack_header _header;
    int _company_num;
    uint64_t _companys[MAX_RET_COMPANY_NUM];
} company2is_get_t;

typedef struct {
    pack_header _header;
    uint64_t _obj_id;
    int _req_type;
} is2company_get_cnt_t;

typedef struct {
    pack_header _header;
    int _company_cnt;
} company2is_get_cnt_t;

/* 2016.08.20: 首页结伴 Timeline 的服务. */
typedef struct {
    pack_header _header;
    int _start_idx;
    int _req_num;
    int _user_num;                            /* 请求中user id数量.    */
    int _user_types[MAX_FOLLOWER_NUM];        /* 类型. 用户:1, 景点:2. */
    uint64_t _user_ids[MAX_FOLLOWER_NUM];     /* 请求时候的ID.         */
} is2company_get_timeline_t;

typedef struct {
    pack_header _header;
    int _company_num;                         /* 返回 company 数量.   */
    int _user_types[MAX_RET_COMPANY_NUM] ;    /* 类型. 用户:1, 景点:2.*/
    uint64_t _user_ids[MAX_RET_COMPANY_NUM] ; /* 请求时候的ID, 返回时用来表明某结伴的归属. */
    uint64_t _companies[MAX_RET_COMPANY_NUM]; /* 返回的文章的ID.      */
} company2is_get_timeline_t;

/* 2016.08.25: 人工为2个结伴增加推荐.    */
typedef struct {
    pack_header _header;
    uint64_t _lhs_id;
    uint64_t _rhs_id;
} is2company_manual_match_t;

typedef struct {
    pack_header _header;
    int _result;                              /* 0: 成功. 1: 失效. 2: 已推荐. 3: 其他错误.  */
} company2is_manual_match_t;

/* 2016.08.28: 人工为2个结伴取消推荐.    */
typedef struct {
    pack_header _header;
    uint64_t _lhs_id;
    uint64_t _rhs_id;
	int _is_two_way;						/* 是否双向移除，0:单向 1:双向 */
} is2company_manual_detach_t;

typedef struct {
    pack_header _header;
    int _result;                              /* 0: 成功. 1: 参数错误. 2: 其他错误.         */
} company2is_manual_detach_t;

/* 2016.08.29: 为结伴人工添加景点.       */
typedef struct {
    pack_header _header;
    uint64_t _company_id ;
    int _new_scenics_num;                     /* 新关联景点的个数.     */
    uint64_t _new_scenics[MAX_SCENICS_NUM_PER_COMPANY-1];
} is2company_manual_attach_scenics_t;

typedef struct {
    pack_header _header;
    int _result;                              /* 0: 成功. 2: 已经饱和. 3: 其他错误.   */
                                              /* 1: 结伴不存在或失效, 但景点关联成功. */
} company2is_manual_attach_scenics_t;

/* 2016.08.16: 文章打赏用户的相关服务. */
typedef struct {
    pack_header _header;
    uint64_t _post_id;
    uint64_t _user_id;
} is2as_set_paiduser_t;

typedef struct {
    pack_header _header;
    char _data[0]; // 返回包中，只需要包头，不需要包体
} as2is_set_paiduser_t;

typedef struct {
    pack_header _header;
    uint64_t _post_id;
    int _start_idx;
    int _req_num;
} is2as_get_paiduser_t;

typedef struct {
    pack_header _header;
    int _user_num;
    uint64_t _user_ids[MAX_PAIDUSER_CNT_PER_POST]; /* 该变量需要保持在结构体最后，数组是实际请求中不会填满 */
} as2is_get_paiduser_t;

typedef struct {
    pack_header _header;
    uint64_t _post_id;
    int _start_idx;
    int _req_num;
} is2as_get_scenic_user_by_post_t;

typedef struct {
    pack_header _header;
    int _scenic_user_num;
    uint64_t _scenic_user_ids[MAX_REF_CNT_PER_POST];
} as2is_get_scenic_user_by_post_t;

/* 2016.10.26: 问答服务相关的接口. */
typedef struct {
	pack_header _header;
    uint64_t _user_id;
    uint64_t _question_id;
    int _invited_user_num;
    uint64_t _invited_users[MAX_QA_INVITED_NUM];
    uint64_t _amount;
    int _scenics_num;
    uint64_t _scenics[MAX_QA_SCENIC_NUM];
} is2qa_set_question_t; // 发布问题

typedef struct {
	pack_header _header;
    int _result;                        /* 0: 成功. 1: 参数不正确. 2: 其他错误. */
} qa2is_set_question_t;

typedef struct {
	pack_header _header;
    uint64_t _q_user_id;
    uint64_t _question_id;
    uint64_t _a_user_id;
    uint64_t _answer_id;
    int _scenics_num;
    uint64_t _scenics[MAX_QA_SCENIC_NUM];
} is2qa_set_answer_t; // 发布答案

typedef struct {
	pack_header _header;
    int _result;                        /* 0: 成功. 1: 参数不正确. 2: 其他错误. */
} qa2is_set_answer_t;

typedef struct {
	pack_header _header;

    /* _qa_type=0: 问题uid; _qa_type=1: 答案uid */
    uint64_t _qa_user_id;
    
    /* 0: 问题. 1: 答案.   */
    int _qa_type;

    uint64_t _question_id;

    /* _qa_type=0: 0; _qa_type=1: 答案id.        */
    uint64_t _answer_id;
    
    /* _qa_type=0: 0; _qa_type=1: 问题uid.       */
    uint64_t _q_user_id;

    /* _qa_type=0: 被邀请者信息; _qa_type=1: 0.  */
    int _invited_user_num;
    uint64_t _invited_users[MAX_QA_INVITED_NUM];

    int _scenics_num;
    uint64_t _scenics[MAX_QA_SCENIC_NUM];
} is2qa_del_qa_t; // 删除问题或答案. 

typedef struct {
	pack_header _header;
    int _result;                        /* 0: 成功. 1: 参数不正确. 2: 其他错误. */
} qa2is_del_qa_t;

typedef struct {
    pack_header _header;
    uint64_t _qa_id;
    int _qa_type;                       /* 0: 问题. 1: 答案.     */
    uint64_t _scenic_id;
    int _attach_detach_type;            /* 0: 关联. 1: 取消关联. */

} is2qa_attach_detach_scenic_t; // 为问答增加(取消)关联旅行地. 

typedef struct {
    pack_header _header;
    int _result;                       /*  0: 成功. 1: 参数不正确. 2: 其他错误. */ 
} qa2is_attach_detach_scenic_t;

typedef struct {
    pack_header _header;
    int _start_idx;
    int _req_num;
    int _user_num;                            /* 请求中user id 数量.    */
    int _user_types[MAX_FOLLOWER_NUM];        /* 类型. 用户:1, 旅行地:2.*/
    uint64_t _user_ids[MAX_FOLLOWER_NUM];     /* 请求时候的ID.          */
} is2qa_get_timeline_t;

typedef struct {
    pack_header _header;
    int _qa_num;                              /* 返回 qa 数量.          */
    int _user_types[MAX_RET_QA_NUM] ;         /* 类型. 用户:1, 旅行地:2.*/
    uint64_t _user_ids[MAX_RET_QA_NUM] ;      /* 请求时候的ID, 返回时用来表明某问答的归属. */
    uint64_t _qas[MAX_RET_QA_NUM];            /* 返回的问答的ID.        */
} qa2is_get_timeline_t;

typedef struct {
    pack_header _header;
    uint64_t _obj_id;
    int _req_type;                            /* 5种类型根据键值请求问答数; 3种类型根据问题请求用户数. */
} is2qa_get_cnt_t;

typedef struct {
    pack_header _header;
    int _qa_cnt;
} qa2is_get_cnt_t;

typedef struct {
	pack_header _header;
    uint64_t _obj_id;
    int _req_type;                            /* 4种类型根据键值请求问答; 3种类型根据问题请求用户.    */
    int _start_idx;
    int _req_num;
} is2qa_get_t;

typedef struct {
	pack_header _header;
    int _obj_num;
    uint64_t _objects[MAX_RET_QA_NUM];        /* 4种类型返回问答列表; 3种类型返回用户列表.            */
} qa2is_get_t;

typedef struct {
	pack_header _header;
    uint64_t _user_id;
    int _start_idx;
    int _req_num;
} is2qa_get_inviteme_t;

typedef struct {
	pack_header _header;
    int _qa_num;
    uint64_t _users_launch_invite[MAX_RET_QA_NUM];
    uint64_t _qas[MAX_RET_QA_NUM];
} qa2is_get_inviteme_t;

typedef struct {
	pack_header _header;
    uint64_t _launch_uid;                     /* 发起邀请者 uid */
    uint64_t _invited_uid;                    /* 被邀请者 uid   */
    uint64_t _question_id;                    /* 问题 id        */

} is2qa_launch_invite_t; // 第三方用户发起邀请回答.

typedef struct {
	pack_header _header;
    int _result;                              /* 0: 完全成功.
                                                 1: 部分成功. 被邀请者近期已答过题了, 不再通知.
                                                 2: 部分成功. 被邀请者近期被其他人邀请过, 尚未作答.
                                                 3: 失败. 参数错误.
                                                 4: 失败. 其他错误.
                                              */
} qa2is_launch_invite_t;

typedef struct {
	pack_header _header;
    uint64_t _follow_uid;                     /* 发起关注者 uid */
    uint64_t _question_id;                    /* 问题 id        */
    uint64_t _q_uid;                          /* 提问者 uid     */

} is2qa_add_follow_q_t; // 关注问题.

typedef struct {
	pack_header _header;
    int _result;                              /* 0: 完全成功.
                                                 1: 部分成功. 之前已经关注过了.
                                                 2: 失败. 参数错误.
                                                 3: 失败. 其他错误.
                                              */
} qa2is_add_follow_q_t;

typedef struct {
	pack_header _header;
    uint64_t _follow_uid;                     /* 取消关注者 uid */
    uint64_t _question_id;                    /* 问题 id        */

} is2qa_del_follow_q_t; // 取消关注问题.

typedef struct {
	pack_header _header;
    int _result;                              /* 0: 完全成功.
                                                 1: 部分成功. 之前并未关注过.
                                                 2: 失败. 参数错误.
                                                 3: 失败. 其他错误.
                                              */
} qa2is_del_follow_q_t;

typedef struct {
	pack_header _header;
    uint64_t _query_uid;                      /* 要查询的 uid */
    uint64_t _question_id;                    /* 问题 id      */

} is2qa_query_follow_q_t; // 是否关注了问题?

typedef struct {
	pack_header _header;
    int _result;                              /* 0: 关注了.
                                                 1: 没关注.
                                                 2: 失败. 参数错误.
                                                 3: 失败. 其他错误.
                                              */
} qa2is_query_follow_q_t;

typedef struct {
    pack_header _header;
    uint64_t _user_id;
} nsreq_set_user_t; // 用户登录后，通知ps记录在线用户

typedef struct {
    pack_header _header;
    char _data[0];
} nsrsp_set_user_t;

typedef struct {
    pack_header _header;
    uint64_t _user_id;
} nsreq_del_user_t; // 用户退出登录，通知ps踢出在线用户

typedef struct {
    pack_header _header;
    char _data[0];
} nsrsp_del_user_t;

#pragma pack()

#endif
