#include <stdio.h>
int main() {
    printf("Size of int : %lu\n", sizeof(int));
    int a[10];
    printf("length of a : %lu\n", sizeof(a) / sizeof(a[0]));
    // 变长数组
    int *pInt = 0;
    printf("length of a : %p", pInt);
    return 0;
}
