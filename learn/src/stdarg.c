#include <stdarg.h>
#include <stdio.h>

void simple_print_int(int begin,...){

	va_list arg_ptr; // 参数指针

	va_start(arg_ptr,begin); // 将参数指针指向 begin 参数

	int j = va_arg(arg_ptr , int ); // 使用 arg_ptr 指针 和 参数类型，来获取参数的值
	
	printf("begin : %p \n",(void*)&begin);
	
	int k = va_arg(arg_ptr , int );

	int l = va_arg(arg_ptr , int );

	printf("%d %d %d \t",j,k,l);
	
	va_end(arg_ptr);

	printf("\n");

}
