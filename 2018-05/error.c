#include "common.h"
#include "lib/ename.inc.c"

#ifdef __GNUC__
__attribute__ ((__noreturn__))
#endif

/*错误处理*/
static void terminate(Boolen useExit3)
{/*{{{*/
    char *s;
    s = getenv("EF_DUMPCORE");
    if(s != NULL && *s != '\0')
        abort();
    else if (useExit3)
        exit(EXIT_FAILURE);
    else
        _exit(EXIT_FAILURE);
}/*}}}*/

static void outputError( Boolen useErr,
                         int err,
                         Boolen flushStdout,
                         const char *format,
                         va_list ap )
{/*{{{*/
    #define BUF_SIZE 500
    char buf[BUF_SIZE * 3];
    char userMsg[BUF_SIZE];
    char errText[BUF_SIZE];

    vsnprintf(userMsg, BUF_SIZE, format, ap);

    if(useErr)
        snprintf( errText, BUF_SIZE, "[%s %s]", (err>0 && err <= MAX_ENAME) ? ename[err] : "?UNKNOWN?", strerror(err) );
    else
        snprintf( errText, BUF_SIZE, ":" );

    snprintf( buf, BUF_SIZE * 3, "ERROR%s %s\n", errText, userMsg );

    if(flushStdout)
        fflush(stdout);
    fputs(buf,stderr);
    fflush(stderr);
}/*}}}*/

void errMsg(const char *format, ...)
{/*{{{*/
    va_list argList;
    int savedErrno;
    savedErrno = errno;
    va_start(argList,format);
    outputError(TRUE,errno,TRUE,format,argList);
    va_end(argList);
    errno = savedErrno;
}/*}}}*/

void errExit( const char *format, ... )
{/*{{{*/
    va_list arglist;
    va_start(arglist, format);
    outputError(TRUE,errno,TRUE,format,arglist);
    va_end(arglist);
    terminate(TRUE);
}/*}}}*/

void err_exit( const char *format, ... )
{/*{{{*/
    va_list arglist;
    va_start(arglist, format);
    outputError(TRUE,errno,TRUE,format,arglist);
    va_end(arglist);
    terminate(FALSE);
}/*}}}*/

void errExitEN( int errnum, const char *format, ... )
{/*{{{*/
    va_list arglist;
    va_start(arglist, format);
    outputError(TRUE,errnum,TRUE,format,arglist);
    va_end(arglist);
    terminate(TRUE);
}/*}}}*/

void fatal( const char *format, ... )
{/*{{{*/
    va_list arglist;
    va_start(arglist,format);
    outputError(FALSE,0,TRUE,format,arglist);
    va_end(arglist);
    terminate(TRUE);
}/*}}}*/

void usageErr(const char *format, ... )
{/*{{{*/
    va_list arglist;
    fflush(stdout);
    fprintf(stderr,"Usage: ");
    va_start(arglist,format);
    vfprintf(stderr,format,arglist);
    va_end(arglist);
    fflush(stderr);
    exit(EXIT_FAILURE);
}/*}}}*/

void cmdLineErr( const char *format, ... )
{/*{{{*/
    va_list arglist;
    fflush(stdout);
    fprintf(stderr,"Command-line usage error:");
    va_start(arglist, format);
    vfprintf(stderr,format,arglist);
    va_end(arglist);
    fflush(stderr);
    exit(EXIT_FAILURE);
}/*}}}*/

