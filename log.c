#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "log.h"

static char* log_levels_str[] = {
    "E",
    "W",
    "I",
    "D"
};

void log_print(int log_level, char *tag, const char *fmt, ...) 
{ 
    if (LOG_LEVEL < log_level)
        return;

    static char fmt_buf[1024];
    static char print_buf[1024];
    va_list args;  
    time_t now;
    time(&now);
    struct tm *now_tm = localtime(&now);
    sprintf(fmt_buf, "[%04d-%02d-%02d %02d:%02d:%02d %s/%s(%d)] %s\n",
           now_tm->tm_year + 1900,
           now_tm->tm_mon + 1 ,
           now_tm->tm_mday,
           now_tm->tm_hour,
           now_tm->tm_min,
           now_tm->tm_sec,
           log_levels_str[log_level],
           tag,
           syscall(SYS_gettid),
           fmt);
           
    va_start(args, fmt_buf);  
    vsprintf(print_buf, fmt_buf, args);
    va_end(args);  

    printf("%s", print_buf);
}  

