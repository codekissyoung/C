#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "common.h"
#include "main.h"
#include "encrypt.h"
#include "question.h"


int main( int argc, char *argv[] )
{
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

