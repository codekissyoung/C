/*
 * 桶排序
 * 桶排序假设输入元素均匀而独立分布在区间[0,1) 即 0 <= x and x < 1;将区间划分成n个相同大小的子区间(桶)，
 * 然后将n个输入按大小分布到各个桶中去，对每个桶内部进行排序。最后将所有桶的排序结果合并起来
 *
 * */

#include <stdio.h>
#include <string.h>

/*
 * 参数说明：
 *     a -- 待排序数组
 *     n -- 数组a的长度
 *     max -- 数组a中最大值的范围
 */
void bucketSort(int a[], int n, int max)
{
    int i,j;
    int buckets[max];

    // 将buckets中的所有数据都初始化为0。
    memset(buckets, 0, max*sizeof(int));

    // 1. 计数
    for(i = 0; i < n; i++)
        buckets[a[i]]++;

    // 2. 排序
    for (i = 0, j = 0; i < max; i++)
    {
        while( (buckets[i]--) >0 )
            a[j++] = i;
    }
}

int main( int argc, char *argv[] )
{
    int a[] = {19,33,4,32,33,42,22,44,54,43,32,211,11,34,32};
    bucketSort( a, sizeof(a) / sizeof(int), 211 );
    for( int i = 0; i < sizeof(a) / sizeof(int); i++ )
        printf("%d ",a[i]);
    return 0;
}
