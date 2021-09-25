#include "func.h"
#include <stdio.h>
#include <stdint.h>

int set()
{
    FUNC_CALLED();
    FUNC_RETURN();
    return 0;
}

int main(){
    printf("%c\n", TOUPPER('d'));
    int64_t a;
    a = -129;
    printf("a : %ld\n", a);
    int *p = NULL;
    PRINT_INT(a);
    TEST(0, "num : %d %d \n", 23,43);
    TEST(1, "num : %d %d \n", 23,43);
    set();
    return 0;
}