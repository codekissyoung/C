#include "common.h"

const int MAX( const int a, const int b)
{
    return a > b ? a : b;
}

void minprintf(char *fmt, ...)
{
    va_list ap;
    char *p, *sval;
    int ival;
    double dval;

    va_start(ap, fmt);// 将ap指向第一个参数
    for( p = fmt; *p; p++ )
    {
        if( *p != '%' )
        {
            putchar(*p);
            continue;
        }
        else
        {
            p++;
            switch( *p )
            {
                case 'd':
                    ival = va_arg( ap, int );
                    printf("%d",ival);
                    break;
                case 'f':
                    dval = va_arg( ap, double );
                    printf("%f",dval);
                    break;
                case 's':
                    for( sval = va_arg( ap, char* ); *sval; sval++ )
                        putchar( *sval );
                    break;
                default:
                    putchar(*p);
                    break;
            }
        }
    }
    va_end(ap);
}


int main(int argc, char *argv[])
{
    minprintf("Hello world %d, %f,%s\n", 10, 893.2233423, "Codekissyoung");
    return 0;
}
