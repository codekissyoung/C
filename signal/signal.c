#include <signal.h>
#include <stdio.h>
#include <unistd.h>
void ouch( int sig )
{
    printf("i got signal %d\n",sig);
    (void)signal(SIGINT,SIG_DFL);
}
int main()
{
    signal(SIGINT,ouch);
    while(1)
    {
        printf("HELLO WORLD\n");
        sleep(1);
    }
    return 0;
}
