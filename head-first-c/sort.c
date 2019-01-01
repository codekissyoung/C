#include "sort.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

int compare_scores( const void* score_a, const void* score_b )
{
    return *(int*)score_a - *(int*)score_b;
}

int compare_areas( const void *a, const void *b )
{
   rectangle* ra = (rectangle*)a;
   rectangle* rb = (rectangle*)b;

   int area_a = (ra->width * ra->height);
   int area_b = (rb->width * rb->height);
   
   return area_a - area_b;
}

int compare_names( const void* a, const void* b )
{
    return strcmp( *(char**)a, *(char**)b );
}

void dump( response r )
{
    printf("Dear %s, you are dump!\n", r.name );
}

void second_chance( response r )
{
    printf("Dear %s, i will give you second chance \n", r.name);
}

void marriage( response r )
{
    printf("Dear %s, i will get marriage with you !\n", r.name );
}

uint64_t time_from_uuid(uint64_t uuid)
{
    uint64_t EPOCH = 1420864633000;

    uint64_t truncation = uuid & 0x7FFFFFFFFFC00000 ;
    uint64_t shifted_timestamp_in_ms = truncation >> 22 ;
    uint64_t unix_timestamp = ( shifted_timestamp_in_ms + EPOCH ) / 1000 ;

    return unix_timestamp;
}

#define MAX_LINE_NUM 1000
#define MAX_ELEMENT_NUM_IN_A_LINE 200

int multi_merge(uint64_t in_list[][MAX_ELEMENT_NUM_IN_A_LINE],
                int in_list_num, 
                uint64_t *out_list,
                int out_list_size,
                int *in_list_x, 
                int *in_list_y,
                int start_idx, 
                int req_len)
{
    int      saved_len        = 0;
    int      merged_len       = 0;
    int      curr_idx_in_list = 0;
    uint64_t max_id           = 0;
    uint64_t max_time         = 0;
    int      max_idx          = -1;
    uint64_t last_merged_id   = 0;

    int idxs[MAX_LINE_NUM];


    memset( idxs, 0, sizeof(idxs[0]) * in_list_num );
    
    while (saved_len < req_len)
	{
        max_time = 0;
        max_idx = -1;

        for (int i = 0; i < in_list_num; i++)
        {
			
			// 第 i 路链表待扫描元素的索引.
            curr_idx_in_list = idxs[i];

			// 每路链表的末尾以0元素值代表结束.
            if (in_list[i][curr_idx_in_list] == 0)
                continue;

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