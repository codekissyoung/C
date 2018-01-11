int printf( const char* format, ... );
int global_init_var = 84;
int global_init_uninit_var;

void func1( int i )
{
    printf("%d\n", i );
}

// 编译时: 将全局变量和函数放在自定义段
__attribute__((section("FOO"))) int global = 42;
__attribute__((section("BAR"))) void foo()
{
    // do nothing
}

int main( void )
{
    static int static_var = 85;
    static int static_var2;
    int a = 1;
    int b;
    func1( static_var + static_var2 + a + b );

    return a;
}

