#include "func.h"
#include <stdio.h>
#include <assert.h>
#include <time.h>

char* date()
{
    static char string[20]; // 位于静态区，程序结束时由系统进行释放
    time_t rawTime;
    time( &rawTime );
    strftime(string, 20, "%Y-%m-%d %H:%M:%S", localtime(&rawTime ));
    return string;
}

void echo(int i)
{
    printf("%d\n",i);
}

void echostr(char *str)
{
    printf("%s\n",str);
}

void swap(int *a, int *b){
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void quickSort(int arrSize, int arr[])
{
    assert( arrSize >=0 );
    if(arrSize == 1 || arrSize == 0)
        return;
    if(arrSize == 2)
    {
        if(arr[0] > arr[1])
            swap(&arr[0], &arr[1]);
        return;
    }

    // 左边 <= split < 右边
    // arrSize >= 3
    int split = (arr[0] + arr[arrSize - 1] + arr[ (int)((arrSize - 1) / 2)]) / 3;
    int left = 0; // 左游标
    int right = arrSize - 1; // 右游标

    while( left < right )
    {
        while ( left < right ) {
            if(arr[right] <= split) {
//                printf("right arr[%d] => %d \n", right, arr[right]);
                break; // 找到一个能放到左边的数
            }
//            printf("skip arr[%d]\n", right);
            right--;
        }
        while( left < right ){
            if (arr[left] > split ) {
//                printf("left arr[%d] => %d \n", left, arr[left]);
                break; // 找到一个能放到右边的数
            }
//            printf("skip arr[%d] => %d\n", left, arr[left]);
            left++;
        }

        // 交换
        if(left < right)
        {
//            printf("swap : arr[%d] %d <=> arr[%d] %d \n", left, arr[left], right, arr[right]);
            swap(&arr[left], &arr[right]);

            left++; // 移动左标
            if( left < right ){ // 确定没碰头之后，移动右标
                right--;
            }
        }
    }
//    printf("left: %d , right %d\n", left, right);

    assert( left == right );
    int mid = left;
    printf("size: %d , split: %d , mid : %d, mid/size: %.2f \n", arrSize, split, mid, (float) mid / arrSize);
    // arr[left] = arr[right] < split, 归到左边计算
    if( arr[mid] < split ){
        quickSort(mid + 1, &arr[0]); // [0, mid]
        quickSort(arrSize - (mid +1), &arr[mid+1]); // [mid + 1, arrSize-1]
    }
    // split < arr[left] = arr[right] 归到右边计算
    if( arr[mid] > split ) {
        quickSort(mid, &arr[0]); // [0, mid -1]
        quickSort(arrSize - mid, &arr[mid]); // [mid, arrSzie -1]
    }
    // 中间项不再参与递归
    if( arr[mid] == split ){
        quickSort( mid, &arr[0]); // [0, mid - 1]
        quickSort( arrSize - (mid+1), &arr[mid+1]); // [mid+1, arrSize -1]
    }
}

void quickSortDemo()
{
    int arr[] = {1999,1,8,72,4,9,33,8,4,4,6,90,83,2,24,-12,-23,123,51,1,21,1,2,34,56,98,0,0,0,0,0,0,34};
    int arrSize = sizeof(arr) /sizeof (int);
    quickSort(arrSize, arr);
    for (int i = 0; i < arrSize; ++i){
        printf("%d,", arr[i]);
    }
}

// 死循环例子
void deadLoopDemo()
{
    FILE *fp;
    unsigned char c;
    fp = fopen("data/test.bin", "rb");
    // EOF = 0xffffffff (-1)
    // 当文件到达末尾时，fgetc(fp) 返回 EOF (-1)
    // 然后由于 c 是 unsigned char 类型，所以存值为 0xff
    // 然后 c != EOF 对比时，又转换为 0x000000ff 与 EOF (-1) 对比
    // 然后程序就进入死循环了
    while ((c = fgetc(fp)) != EOF){
        putchar(c);
    }
    fclose(fp);
}
