#ifndef __COMMON_FUNC_H_
#define __COMMON_FUNC_H_

#include <stdint.h>

uint64_t time_from_uuid(uint64_t uuid);

#define MAX_LINE_NUM 1000
#define MAX_ELEMENT_NUM_IN_A_LINE 200

int multi_merge(uint64_t[][MAX_ELEMENT_NUM_IN_A_LINE], int, uint64_t*, int, int *, int *, int, int);

#endif
