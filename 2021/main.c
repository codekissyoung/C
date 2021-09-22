#include "func.h"
#include <stdio.h>

void error_log(char *msg){
    printf("[%s %s]%s:%d %s\n",__DATE__, __TIME__, __FILE__,__LINE__, msg);
}

int main() {
    error_log("try once again!");
}