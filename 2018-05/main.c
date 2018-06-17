#include "common.h"
int main(int ac, char *av[])
{
    struct Test
    {/*{{{*/
        char a;
        double b;
        char c;
    };/*}}}*/
    struct Test t1 = {'a', 892.1333, 'c'};
    printf("%p %p %p sizeof : %d\n",(void*)&t1.a,(void*)&t1.b,(void*)&t1.c, (int)sizeof(t1));
    return 0;
}
