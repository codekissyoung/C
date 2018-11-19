#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <stdint.h>

#include "common.h"
#include "main.h"
#include "encrypt.h"
#include "question.h"
#include "sort.h"
#include "log.h"

int main( int argc, char *argv[] )
{
    Test_arr_struct test_arr_struct;

    printf("sizeof : %ld\n",sizeof(test_arr_struct));
    pid_t fd[2];

    if( pipe( fd ) == -1 )
    {
        printf("open fd error");
        exit(1);
    }

    pid_t pid = fork();

    if( pid == -1 )
    {
        printf("fork error");
        exit(1);
    }

    if( pid == 0 )
    {
        dup2( fd[1], 1 );
        close( fd[0] );

        printf("from child ");
        return 0;        
    }


    dup2( fd[0], 0 );
    close( fd[1] );

    printf("from parent !");

    char line[255];
    fgets( line, 255, stdin );
    printf( "%s", line );
    
/*
    if( execl("/sbin/ifconfig", "/sbin/ifconifg", NULL ) == -1  )
    {
        if( execlp("ifconfig","ifconfig",NULL) == -1 )
        {
            fprintf(stderr,"cannot run ifconfig : %s", strerror(errno));
            return 1;
        }
    }
*/
/*
   char comment[80];
   char cmd[120];

   fgets( comment, 80, stdin );
   sprintf( cmd, "echo '%s %s' >> reports.log", comment, now() );
   system(cmd);
*/
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

