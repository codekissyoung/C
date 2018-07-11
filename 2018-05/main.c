#include "common.h"

const int MAX( const int a, const int b)
{
    return a > b ? a : b;
}

int main(int ac, char *av[])
{
    struct Test
    {
        char a;
        double b;
        char c;
    };

    int c = MAX( 192, 90 );
    printf("c : %d\n", c);
    c = 10000;
    printf("c : %d\n", c);


	const char *pointer_str[5] = {
		"str",
		"string2 ,sdfadfsfssdfasfsa",
		"string3 hdhdhdh",
		"xxixixixi",
		"codsdadfsssss"
	};
	for(int k = 0;k < 5;k++)
		printf("pointer_str[%d] : %p : %s \n",k,pointer_str[k],pointer_str[k]);

	char array_str[5][40] = {
		"sdfaiisisis",
		"xixixiix sss",
		"hahah",
		"codekissyoung"
	};
	for(int p = 0;p < 5;p++)
		printf("array_str[%d] : %p : %s \n",p,array_str[p],array_str[p]);



    struct Test t1 = {'a', 892.1333, 'c'};
    printf("%p %p %p sizeof : %d\n",(void*)&t1.a,(void*)&t1.b,(void*)&t1.c, (int)sizeof(t1));
    return 0;
}
