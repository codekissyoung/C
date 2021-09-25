#include "func.h"
#include <stdio.h>
#include <stdint.h>

//int main()
//{
//    FILE *fp;
//    fp=fopen("data/test.bin", "rb");
//    int c;
//    while ((c = getc(fp)) != EOF )
//    {
//        if( c == EOF){
//            printf("c = -1\n");
//        }
//        printf("%c", c);
//    }
//
//    fclose(fp);
//    return 0;
//}

int main(void){
    FILE *fp;
    unsigned char c;
    fp = fopen("data/test.bin", "rb");
    // EOF = 0xffffffff
    // 当文件到达末尾时，fgetc(fp) 返回 EOF
    // 然后由于 c 是 unsigned char 类型，所以存值为 0xff
    // 然后 c != EOF 对比时，又转换为 0x000000ff 与 EOF 对比
    // 然后程序就进入死循环了
    while ((c = fgetc(fp)) != EOF){
        putchar(c);
    }
    fclose(fp);
    return 0;
}