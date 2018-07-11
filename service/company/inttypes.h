#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#if defined __GNUC__
#include_next <inttypes.h>
#elif defined _MSC_VER
#include "inttypes_vc.h"
#else
#error Unknown compiler
#endif
