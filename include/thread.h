#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

extern void *first_thread(void *arg);
extern void *second_thread(void *arg);
extern void *third_thread(void *arg);
