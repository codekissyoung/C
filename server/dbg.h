#ifndef _dbg_h_
#define _dbg_h_
/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <errno.h>
#include <string.h>

#define error_str ( errno == 0 ? "" : strerror(errno) )
#define dbg_log(level,msg,...) \
fprintf(stderr,"[" level "] " msg " %s (%s:%d)\n", ##__VA_ARGS__, error_str,__FILE__, __LINE__ )

#ifdef NDEBUG
    #define debug(msg,...)
#else
    #define debug(msg,...) dbg_log("dbg", msg, ##__VA_ARGS__)
#endif

#define log_err(msg,...) dbg_log("error", msg, ##__VA_ARGS__)
#define log_warn(msg,...) dbg_log("warning", msg, ##__VA_ARGS__)
#define log_info(msg,...) dbg_log("info", msg, ##__VA_ARGS__)

#define check(cond,msg,...) if(!(cond)) {log_err(msg,##__VA_ARGS__); errno=0; goto error;}
#define check_mem(cond) check((cond),"out of memory")

// 用于一些不可能执行到的分支，如果执行了，那就是代码逻辑有问题
#define impossible(msg,...) dbg_log("imp", msg, ##__VA_ARGS__); errno=0; goto error;

/////////////////////////////////////////////////////////////////////////////
#endif