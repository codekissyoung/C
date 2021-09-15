#include <stdio.h>

void print( int a ){
	printf("%d\n", a);
}

int main()
{
	(*print)(9);

	void (*print_ptr1)(int) = &print;
	void (*print_ptr2)(int) = print;

	print_ptr1(20);
	print_ptr2(30);

	printf("%u\n", print_ptr1 == print_ptr2);

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
