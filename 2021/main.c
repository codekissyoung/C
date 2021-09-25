#include "func.h"
#include <stdio.h>
#include <stdint.h>

int main()
{
    printf("%c %c\n", TOUPPER('d'), TOLOWER('H'));
    int64_t a;
    a = -129;
    printf("a : %ld\n", a);
    int *p = NULL;
    PRINT_INT(a);
    TEST(0, "num : %d %d \n", 23,43);
    TEST(1, "num : %d %d \n", 23,43);
    error_log("try once again!");
    return 0;
}