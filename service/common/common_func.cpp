#include <cstdlib>
#include <cstring>

#include "common_func.h"

uint64_t time_from_uuid(uint64_t uuid)
{
    uint64_t EPOCH = 1420864633000;

    uint64_t truncation = uuid & 0x7FFFFFFFFFC00000 ;
    uint64_t shifted_timestamp_in_ms = truncation >> 22 ;
    uint64_t unix_timestamp = ( shifted_timestamp_in_ms + EPOCH ) / 1000 ;

    return unix_timestamp;
}

/*
 * 多路归并算法. 这是一个古典算法, 但也包含了部分青驿业务逻辑.
 *
 * 每一路链表元素内部有序, 依次扫描每一路的元素. 扫描过程中保留该路下一个可能
 * 选出元素的索引, 一旦被选中, 索引向后更新一位.
 *
 * 假如来自于不同路的元素值相等, 古典算法中并未给出排序的唯一答案. 本算法中结
 * 合青驿实际使用场景, 采用了排除重复的方式, 放弃后出现的同值元素.
 *
 * 例如, 青驿用户在首页 Timeline 中可拉取到好友和旅行地的推送文章. 假设来自于
 * 某旅行地的文章恰好同是他关注的好友所发布的, 那么该用户将拉取到2篇完全一样的
 * 文章在相邻的位置上, 而这是违背了产品初衷的.
 *
 * 另外, 在比较2个元素的先后顺序时, 青驿的基准是元素ID对应的文章的发布时间. 由
 * 于本算法没有额外引入这类参数, 所以使用的是基于元素值反向推算出该时间.
 *
 */

int multi_merge(uint64_t in_list[][MAX_ELEMENT_NUM_IN_A_LINE], int in_list_num, 
                uint64_t *out_list, int out_list_size, int *in_list_x, int *in_list_y,
                int start_idx, int req_len)
{
    if (in_list == NULL || out_list == NULL || out_list_size < req_len)
        return -1;

    int saved_len = 0, merged_len = 0;
    int curr_idx_in_list = 0;
    
    int idxs[MAX_LINE_NUM];
    memset(idxs, 0, sizeof(idxs[0]) * in_list_num);
    
    uint64_t max_id = 0;
    uint64_t max_time = 0;
    int max_idx = -1;

    uint64_t last_merged_id = 0 ;

    while (saved_len < req_len)
	{
        max_time = 0;
        max_idx = -1;

        for (int i = 0; i < in_list_num; i++) {
			
			// 第 i 路链表待扫描元素的索引.
            curr_idx_in_list = idxs[i];

			// 每路链表的末尾以0元素值代表结束.
            if (in_list[i][curr_idx_in_list] == 0) {
                continue;
            }

            uint64_t time = time_from_uuid(in_list[i][curr_idx_in_list]);
            if (time > max_time)
			{
                max_time = time;
                max_id = in_list[i][curr_idx_in_list];
                max_idx = i;
            }
        }

        if (max_idx == -1) 
            break;

		// 将当前选出元素所在链表的待扫描索引下移一位.
        idxs[max_idx]++;
        
        // 本归并排序算法不支持出现2个值相同的元素. 跳过.
        if ( max_id == last_merged_id )
            continue ;
        last_merged_id = max_id ;

        // 查看当前候选元素的索引是否满足要求.
        if (merged_len++ < start_idx) 
            continue;

		// 选出符合条件的元素.
        in_list_x[saved_len] = max_idx ;
        in_list_y[saved_len] = idxs[max_idx] - 1 ;
        out_list[saved_len]  = max_id;

        saved_len ++ ;
    }

    // 结尾标志
    out_list[saved_len] = 0;

    return saved_len;
}
