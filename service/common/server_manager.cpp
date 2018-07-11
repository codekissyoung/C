#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server_manager.h"
#include "log.h"
#include "conf.h"

static int _load_range_table(range_table_t *table, FILE *fp)
{
    if (table == NULL || fp == NULL) {
		log_txt_err("table == NULL \n");
        return  -1;
    }

    int size = 8;
    table->_segments = (range_item_t *)calloc(size, sizeof(range_item_t));
    if (table->_segments == NULL) {
		log_txt_err("calloc error\n");
        return -1;
    }

    char line[1024];
    uint64_t min, max, old_max = 0;
    int svr_idx = 0;
    while (fgets(line, sizeof(line), fp)) {
        sscanf(line, "%ju\t%ju", &min, &max);
        if (max == 0) {
            max = 0xffffffffffffffffllu;
        }

        if (svr_idx < size) {
            if (min < old_max) {
                log_txt_err("range file incorrect!, line[%d] min[%ju] is "
                            "smaller than preline max[%ju]", svr_idx+1, min, old_max);
            }

            old_max = max;
            table->_segments[svr_idx]._min = min;
            table->_segments[svr_idx]._max = max;
            svr_idx++;
            continue;
        }

        size *= 2;
        table->_segments = (range_item_t *)realloc(table->_segments, size * sizeof(range_item_t));
        if (table->_segments == NULL) {
            log_txt_err("realloc memorry for range table failed, new size:[%zu]", 
                        size * sizeof(range_item_t));
            return -1;
        }
    }

    table->_segment_num = svr_idx;
 
    return 0;
}

/* 初始化区间表 */
range_table_t *range_table_init(const char *range_file)
{
    if (range_file == NULL) {
			return NULL;
    }

    FILE *fp = fopen(range_file, "r");
    if (fp == NULL) {
        log_txt_err("open file failed!, file_path[%s]", range_file);
        return NULL;
    }

    range_table_t *table = (range_table_t *)calloc(1, sizeof(range_table_t));
    if (table == NULL) {
        fclose(fp);
        return NULL;
    }

    if (_load_range_table(table, fp) < 0) {
        free(table);
        fclose(fp);
        return NULL;
    }

    return table;
}

/* 查找key所在区间 */
int range_table_find(range_table_t *table, uint64_t key)
{
	if (table == NULL)
	{
    	log_txt_err("range_table is null");
		return -1 ;
	}
    
	int left = 0;
    int right = table->_segment_num;
    int mid = 0;
    while (left < right) {
        mid = (left + right) / 2;
        if (table->_segments[mid]._min > key) {
            right = mid;
            continue;
        }
        else if (table->_segments[mid]._max < key) {
            left = mid;
            continue;
        }
        else {
            return mid;
        }
    }

    return -1;
}

/* 销毁区间表 */
int range_table_uninit(range_table_t *table)
{
    if (table) {
        if (table->_segments) {
            free(table->_segments);
            table->_segments = NULL;
        }

        free(table);
        table = NULL;
    }

    return 0;
}

/* 初始化svr_mgr */
svr_mgr_t *sm_init(const char *conf_file, open_conn_func_t open_func, close_conn_func_t close_func)
{
    int ret = 0;
    ret = conf_init(conf_file);
    if (ret < 0) {
        log_txt_err("read conf file failed, conf:[%s]", conf_file);
        return NULL;
    }

    char svr_name[256] = {'\0'};
    read_conf_str("", "SERVICE_NAME", svr_name, sizeof(svr_name), "");
    if (svr_name[0] == 0x00) {
        log_txt_err("SERVICE_NAME was not config or it's null");
        return NULL;
    }

    svr_mgr_t *mgr = (svr_mgr_t *)calloc(1, sizeof(svr_mgr_t));
    if (mgr == NULL) {
        log_txt_err("calloc memory for svr_mgr_t failed");
        return NULL;
    }
    mgr->_open_func = open_func;
    mgr->_close_func = close_func;

    char section[128];
    char conf_name[128];

    // 服务切片数
    mgr->_group_num = 0;
    read_conf_int(svr_name, "GROUP_NUM", &(mgr->_group_num), 0);
    if (mgr->_group_num == 0) {
        log_txt_err("GROUP_NUM was not config or it's 0");
        sm_uninit(mgr);
        return NULL;
    }

    // 切片类型
    mgr->_sharding_type = -1;
    read_conf_int(svr_name, "SHARDING_TYPE", &(mgr->_sharding_type), 0);
    if (mgr->_sharding_type == -1) {
        log_txt_err("SHARDING_TYPE was not config or it's -1");
        sm_uninit(mgr);
        return NULL;
    }

    // 如果是按照区间切片，读取区间表
    if (mgr->_sharding_type == SHARDING_BY_RANGE) {
        read_conf_str(svr_name, "RANGE_TABLE", mgr->_range_file, 
                      sizeof(mgr->_range_file), "");
        if (mgr->_range_file[0] == 0x00) {
            log_txt_err("RANGE_TABLE was not config or it's null");
            sm_uninit(mgr);
            return NULL;
        }

        mgr->_range_table = range_table_init(mgr->_range_file);

		if (mgr->_range_table->_segment_num != mgr->_group_num)
		{
			log_txt_err("RANGE_TABLE line num:%d, GROUP_NUM:%d",mgr->_range_table->_segment_num,mgr->_group_num) ;
			sm_uninit(mgr);
			return NULL;
		}
    }

    mgr->_groups = (svr_group_t *)calloc(mgr->_group_num, sizeof(svr_group_t));
    if (mgr->_groups == NULL) {
        log_txt_err("calloc memory failed, size:%d", (int)(mgr->_group_num * sizeof(svr_group_t)));
        sm_uninit(mgr);
        return NULL;
    }

    for (int i = 0; i < mgr->_group_num; i++) {
        svr_group_t *svr_ptr = mgr->_groups + i;
        snprintf(section, sizeof(section), "%s_GROUP_%d", svr_name, i);
        snprintf(conf_name, sizeof(conf_name), "%s_NODE_NUM", svr_name);

        read_conf_int(section, conf_name, &(svr_ptr->_bak_num), 0);
        if (svr_ptr->_bak_num == 0) {
            log_txt_err("[%s]->%s not config or is's 0", section, conf_name);
            sm_uninit(mgr);
            return NULL;
        }

        svr_ptr->_cur_idx = -1;
        svr_ptr->_cur_conn = NULL;
        for (int j = 0; j < svr_ptr->_bak_num; j++) {
            /* get host */
            snprintf(conf_name, sizeof(conf_name), "%s_HOST_%d", svr_name, j);
            read_conf_str(section, conf_name, svr_ptr->_svrs[j]._host, 
                          sizeof(svr_ptr->_svrs[j]._host), "");
            if (svr_ptr->_svrs[j]._host[0] == 0x00) {
                log_txt_err("[%s]->%s not config or it's null", section, conf_name);
                sm_uninit(mgr);
                return NULL;
            }

            /* get port */
            snprintf(conf_name, sizeof(conf_name), "%s_PORT_%d", svr_name, j);
            read_conf_int(section, conf_name, (int *)&(svr_ptr->_svrs[j]._port), 0);
            if (svr_ptr->_svrs[j]._port == 0x00) {
                log_txt_err("[%s]->%s not config or it's 0", section, conf_name);
                sm_uninit(mgr);
                return NULL;
            }

            /* create connection */
            ret = open_func(svr_ptr->_svrs[j]._host, svr_ptr->_svrs[j]._port, &(svr_ptr->_svrs[j]._conn));
            if (ret < 0 || svr_ptr->_svrs[j]._conn == NULL) {
                log_txt_err("create connect to [%s:%d] failed", 
                            svr_ptr->_svrs[j]._host, svr_ptr->_svrs[j]._port);
                svr_ptr->_health_flag[j] = -1;
            }
            else {
                if (svr_ptr->_cur_idx == -1) {
                    svr_ptr->_cur_idx = j;
                    svr_ptr->_cur_conn = svr_ptr->_svrs[j]._conn;
                }

                svr_ptr->_health_flag[j] = 0;
            }
        }
    }

    return mgr;
}

static int _check_svr(svr_group_t *svr)
{
    if (svr->_cur_conn == NULL) {
        return -1;
    }

    return 0;
}

static svr_group_t *_sharding_by_mod(svr_mgr_t *svr_mgr, uint64_t sharding_key)
{
    svr_group_t *svr = NULL;
    int idx = sharding_key % svr_mgr->_group_num;
    svr = svr_mgr->_groups + idx;

    if (_check_svr(svr) < 0) {
        return NULL;
    }

    return svr;
}
            
static svr_group_t *_sharding_by_range(svr_mgr_t *svr_mgr, uint64_t sharding_key)
{
    if (svr_mgr->_range_table == NULL) {
        log_txt_err("sharding by range bug range_table is null, key:[%ju]", sharding_key);
        return NULL;
    }

    int idx = range_table_find(svr_mgr->_range_table, sharding_key);
    if (idx < 0) {
        log_txt_err("key[%ju] not found in range_table[%s]", sharding_key, svr_mgr->_range_file);
        return NULL;
    }

    svr_group_t *svr = svr_mgr->_groups + idx;
    if (_check_svr(svr) < 0) {
        return NULL;
    }

    return svr;
}

/* 从svr_mgr获取svr */
svr_group_t *sm_get_svr(svr_mgr_t *svr_mgr, uint64_t sharding_key)
{
    if (svr_mgr == NULL) {
        return NULL;
    }

    svr_group_t *svr = NULL;
    switch (svr_mgr->_sharding_type) {
        case SHARDING_BY_MOD:
            svr = _sharding_by_mod(svr_mgr, sharding_key);
            break;
        case SHARDING_BY_RANGE:
            svr = _sharding_by_range(svr_mgr, sharding_key);
            break;
    }

    return svr;
}

/* 处理down机问题 */
int sm_deal_down_svr(svr_mgr_t *svr_mgr, svr_group_t *svr)
{
    if (svr == NULL) {
        return -1;
    }

    if (svr->_cur_conn != NULL) {
        svr_mgr->_close_func(svr->_cur_conn);
        svr->_cur_conn = NULL;
    }
    if (svr->_cur_idx > -1) {
        svr->_health_flag[svr->_cur_idx] = -1;
        svr->_svrs[svr->_cur_idx]._conn = NULL;
    }

    while (svr->_cur_idx + 1 < svr->_bak_num) {
        svr->_cur_idx += 1;
        if (svr->_health_flag[svr->_cur_idx] == 0) {
            svr->_cur_conn = svr->_svrs[svr->_cur_idx]._conn;
            return 0;
        }
    }

    svr->_cur_conn = NULL;
    svr->_cur_idx = -1;
    int ret = 0;
    for (int i = 0; i < svr->_bak_num; i++) {
        ret = svr_mgr->_open_func(svr->_svrs[i]._host, 
                                  svr->_svrs[i]._port, &(svr->_svrs[i]._conn));
        if (ret < 0) {
            log_txt_err("reconnet [%s:%d] failed!", svr->_svrs[i]._host, 
                        svr->_svrs[i]._port);
            svr->_svrs[i]._conn = NULL;
            svr->_health_flag[i] = -1;
        }
        else {
            if (svr->_cur_idx == -1) {
                svr->_cur_idx = i;
                svr->_cur_conn = svr->_svrs[i]._conn;
            }

            svr->_health_flag[i] = 0;
        }
    }

    return 0;
}

/* 重新建立连接 */
int sm_reconnect(svr_mgr_t *svr_mgr, svr_group_t *svr)
{
    if (svr->_cur_conn != NULL) {
        svr_mgr->_close_func(svr->_cur_conn);
        svr->_cur_conn = NULL;
    }
    int idx = svr->_cur_idx;
    int ret = svr_mgr->_open_func(svr->_svrs[idx]._host, svr->_svrs[idx]._port, &(svr->_svrs[idx]._conn));
    if (ret < 0) {
        log_txt_err("reconnet failed when deal over time, host:[%s] port:[%d]", 
                    svr->_svrs[idx]._host, svr->_svrs[idx]._port);
        sm_deal_down_svr(svr_mgr, svr);
    }

    return 0;
}

/* 处理超时 */
int sm_deal_overtime(svr_mgr_t *svr_mgr, svr_group_t *svr)
{
    return sm_reconnect(svr_mgr, svr);
}

/* 销毁svr_mgr */
int sm_uninit(svr_mgr_t *svr_mgr)
{
    if (svr_mgr == NULL) {
        return 0;
    }

    if (svr_mgr->_groups == NULL) {
        free(svr_mgr);
        return 0;
    }

    for (int i = 0; i < svr_mgr->_group_num; i++) {
        svr_group_t *svr = svr_mgr->_groups + i;
        for (int j = 0; j < svr->_bak_num; j++) {
            if (svr->_svrs[j]._conn != NULL) {
                svr_mgr->_close_func(svr->_svrs[j]._conn);
                svr->_svrs[j]._conn = NULL;
            }
        }
    }

    free(svr_mgr->_groups);
    svr_mgr->_groups = NULL;
    free(svr_mgr);

    return 0;
}

