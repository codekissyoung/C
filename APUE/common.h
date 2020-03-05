#ifndef __COMMON_H_
#define __COMMON_H_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define min(m,n) ((m) < (n)) ? (m) : (n)
#define max(m,n) ((m) > (n)) ? (m) : (n)
#define MODE_RWRWRW S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH

typedef enum Boolean { FALSE, TRUE } bool;

#endif