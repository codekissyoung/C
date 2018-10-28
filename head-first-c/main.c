#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "common.h"
#include "main.h"
#include "encrypt.h"

int main( int argc, char *argv[] )
{
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
}

