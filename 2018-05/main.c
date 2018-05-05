#include "common.h"

int add_range(int low, int high)
{
    int i, sum;
    sum = 0;
    for(i = low; i <= high; i++)
        sum = sum + i;
    return sum;
}

int main( int argc, char *argv[] )
{
    int man = 0;
    scanf("%d", &man);

    int result[100];
    result[0] = add_range(1, 10);
    result[1] = add_range(1, 100);
    printf("result[0] : %d, result[1] : %d\n", result[0], result[1]);

    int sum = 0, i = 0;
    char input[5];
    printf("input somthing : ");
    scanf("%s", input);
    for (i = 0; input[i] != '\0'; i++)
        sum = sum * 10 + input[i] - '0';
    printf("input=%d\n",sum);
    return 0;
}
