#include "common.h"
#include "error.c"
#include "func.c"
int main(int ac, char *av[])
{
    int             len = sizeof(struct utmp);
    char            utmpbuf[4*len];

    const char *USERNAME = "codekissyoung";
    struct Books book1 = {
        "《钢铁是如何炼成的》",
        "佚名",
        "保尔柯察金",
        893112
    };
    Simple s1 = {
        329312,
        'b',
        1892.388
    };
    printf("%f\n",s1.c);
    struct NODE node1 = {
        "helloworld",
        NULL
    };
    struct A user_name = {
        NULL,
        "codekissyoung"
    };
    struct B user_age = {
        NULL,
        21
    };
    user_name.pointer = &user_age;
    user_age.pointer = &user_name;

    struct bit_field bt1;
    bt1.a = 3;
    bt1.b = 4;
    bt1.c = 5;
    bt1.d = 6;
    bt1.e = 2;

    union Data d1;
    strcpy( d1.str, "codekissyoung" );

    int utmpfd = open(UTMP_FILE, O_RDONLY);

    DIR *dir_ptr;
    if( (dir_ptr = opendir( "lib" )) == NULL )
        oops("opendir","Error");

    struct dirent *direntp;
    while( (direntp = readdir( dir_ptr )) != NULL )
    {
        printf("%s\n",direntp -> d_name );
    }

    struct stat infobuf;
    if( stat("main.c",&infobuf) == -1 )
        oops("stat","Error");
    else
        printf("infobuf.size : %ld\n", infobuf.st_size);

    closedir(dir_ptr);

    read(utmpfd, utmpbuf, 4 * len);
    struct utmp *recive  = (struct utmp *)&utmpbuf[0];
    close(utmpfd);
    test_static();
    test_static();
    test_static();
    // cp 代码
    /*
    int in_fd;
    int out_fd;
    int n_chars;
    char buf[BUFSIZE];

    in_fd = open( av[1], O_RDONLY );

    out_fd = creat( av[2], 0644 );
    if( out_fd == -1 )
        oops("outfd error", av[2]);

    while( (n_chars = read(in_fd, buf, BUFSIZE)) > 0 )
    {
        if( write(out_fd, buf, n_chars) != n_chars )
            oops("写入出错",av[2]);
    }
    if( n_chars == -1 )
        oops("读取出错", av[2]);

    close(in_fd);
    close(out_fd);
    */
    return 0;
}
