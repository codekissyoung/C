#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <stdint.h>
#include <dirent.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/termios.h>
#include <sys/types.h>

#include "common.h"
#include "main.h"
#include "encrypt.h"
#include "question.h"
#include "sort.h"
#include "log.h"

#define MAXLINE 4096
#define _XOPEN_SOURCE 600
#define FILE_MODE ( S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH )
#define DIR_MODE ( FILE_MODE | S_XUSR | S_IXGRP | S_IXOTH )
#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

// 设置文件状态标志函数
void set_fl( int fd, int flags )
{
    int val = fcntl( fd, F_GETFL, 0);
    if( val < 0 )
    {
        perror("error");
        exit(errno);
    }

    val |= flags;

    if( fcntl( fd, F_SETFL, val ) < 0 )
    {
        perror("error");
        exit(errno);
    }
}

int main( int argc, char *argv[] )
{   
    int scores[] = {23.242,322,453,564,434,567,239};
    qsort( scores, sizeof(scores) / sizeof(int), sizeof(int), compare_scores );
    for( int i = 0; i < sizeof(scores) / sizeof(int); i++ )
    {
        printf("%d\t",scores[i]);
    }
    printf("\n");

    char *names[] = {"Dark","Maru","inovation","Classic","showtime","byun","tyty"};
    qsort( names, 7, sizeof(char*), compare_names );
    for( int i = 0; i < 7; i++ )
    {
        printf("%s\t", names[i] );
    }    
    printf("\n");


    void ( *replies[] )( response ) = {dump, second_chance, marriage };
    response r[] = {
        {"Maru",DUMP},
        {"Classic",SECOND_CHANCE},
        {"Byun",MARRIAGE}
    };

    for( int i = 0; i < 3; i ++ )
    {
        (replies[r[i].type])(r[i]);
    }
    return 0;
    /*
    char question[80];
    char suspect[20];

    node * start_node = create("Does suspect have a mustache");
    start_node -> no = create("Loretta Barnsworth");
    start_node -> yes = create("Vinny the Spoon");

    node *current;
    do
    {
        current = start_node;

        while(1)
        {
            if( yes_no(current->question) )
            {
                if( current -> yes )
                    current = current -> yes;
                else
                {
                    printf("SUSPECT IDENTIFIED\n");
                    break;
                }
            }
            else if( current -> no )
            {
                    current = current -> no;
            }
            else
            {
                printf("Who is the suspect?");
                fgets( suspect, 20, stdin );
                node *yes_node = create( suspect );
                current -> yes = yes_node;

                node *no_node = create(current->question);
                current -> no = no_node;

                printf("Give me a quesion that is ture for %s but not for %s?",suspect, current->question );
                fgets(question, 80, stdin);
                free(current->question);
                current->question = strdup(question);
                break;
            }
        }
    }
    while( yes_no( "Run again") );

    release( start_node );
    return 0;
    */

    /*
    // optarg
    char *delivery = "";
    int thick = 0;
    int count = 0;
    char ch;
    struct fish snappy = {"Snappy","鲑鱼",69,4,{"肉",7.5}};

    printf("%s\n",snappy.name);
    
    while( (ch = getopt(argc,argv,"d:t")) != EOF )
    {
        switch(ch)
        {
            case 'd':
                delivery = optarg;
                break;

            case 't':
                thick = 1;
                break;

            default:
                fprintf(stderr,"Unknown option: '%s'\n",optarg);
                return 1;
        }
    }

    argc -= optind;
    argv += optind;

    if(thick)
        puts("Thick crust.");

    if(delivery[0])
        printf("To be delivered %s.\n",delivery);

    puts("Ingredients:");
    for(count = 0; count < argc; count++)
    {
        encrypt(argv[count]);
        puts(argv[count]);
    }
    return 0;
    */
}
