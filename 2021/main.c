#include <stdio.h>
#include <stdarg.h>

void print(int a) {
    printf("%d\n", a);
}

double average(int arg_num, ...) {
    va_list ap; // 固定格式
    va_start(ap, arg_num);// 固定格式

    double sum = 0;
    for (int j = 0; j < arg_num; j++) {
        // 每次运行 va_arg，都返回一个入参，double 指明解析方式
        sum += va_arg(ap, double);　// 固定格式
    }

    va_end(ap);　// 固定格式
    return sum / arg_num;
}

int main() {
    (&print)(9);

    void (*print_ptr1)(int) = &print;
    void (*print_ptr2)(int) = print;

    print_ptr1(20);
    print_ptr2(30);

    printf("%u\n", print_ptr1 == print_ptr2);
    printf("%f\n", average(2, (double) 4, (double) 8));

/*
	printf("Size of int : %lu\n", sizeof(int));
    int a[10];
    printf("length of a : %lu\n", sizeof(a) / sizeof(a[0]));
    // 变长数组
    int *pInt = 0;
    printf("length of a : %p", pInt);
*/
    return 0;
}
