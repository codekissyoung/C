#ifndef __SERVER_MANAGER_H_
#define __SERVER_MANAGER_H_

#include "common.h"
#include <stdint.h>

typedef int (*open_conn_func_t)(char *host, short port, void **conn);
typedef int (*close_conn_func_t)(void *conn);

typedef enum {
    SHARDING_BY_MOD = 1,
    SHARDING_BY_RANGE = 2,

    SHARDING_END
} sharding_type_t;

typedef struct {
    uint64_t _min;
    uint64_t _max;
} range_item_t;

typedef struct {
    int _segment_num;
    range_item_t *_segments;
} range_table_t;

typedef struct {
    char _host[MAX_SVR_HOST_LEN];
    short _port;
    union {
        void *_conn;
        int _socket;
    };
} svr_single_t;

typedef struct {
    int _bak_num;
    svr_single_t _svrs[MAX_NODE_NUM_PER_GROUP];

    char _health_flag[MAX_NODE_NUM_PER_GROUP];
    int _cur_idx;
    union {
        void *_cur_conn;
        int _socket;
    };

    char *_req_buff;
    char *_rsp_buff;
    int _req_buff_len;
    int _rsp_buff_len;
    int _left_len;
} svr_group_t;

typedef struct {
    int _group_num;
    svr_group_t *_groups;

    int _sharding_type; 
    range_table_t *_range_table;
    char _range_file[MAX_FILE_PATH_LEN];

    open_conn_func_t _open_func;
    close_conn_func_t _close_func;
} svr_mgr_t;

/* 初始化svr_mgr */
svr_mgr_t *sm_init(const char *conf_file, open_conn_func_t open_func, close_conn_func_t close_func);

/* 从svr_mgr获取svr */
svr_group_t *sm_get_svr(svr_mgr_t *svr_mgr, uint64_t sharding_key);

/* 处理down机问题 */
int sm_deal_down_svr(svr_mgr_t *svr_mgr, svr_group_t *svr);

/* 处理超时 */
int sm_deal_overtime(svr_mgr_t *svr_mgr, svr_group_t *svr);

/* 重新建立连接 */
int sm_reconnect(svr_mgr_t *svr_mgr, svr_group_t *svr);

/* 销毁svr_mgr */
int sm_uninit(svr_mgr_t *svr_mgr);

/* 初始化区间表 */
range_table_t *range_table_init(const char *range_file);

/* 查找key所在区间 */
int range_table_find(range_table_t *table, uint64_t key);

/* 销毁区间表 */
int range_table_uninit(range_table_t *table);

#endif

