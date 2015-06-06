#ifndef _LOG_H_
#define _LOG_H_

#define LOG_LEVEL_ERR 0
#define LOG_LEVEL_WARNING 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_DEBUG 3

#define LOG_LEVEL 3 //print all <= 3's log

#define LOG_TAG __FILE__

#define LOG_ERR(...) log_print(LOG_LEVEL_ERR, LOG_TAG, ##__VA_ARGS__)
#define LOG_WARNING(...) log_print(LOG_LEVEL_WARNING, LOG_TAG, ##__VA_ARGS__)
#define LOG_INFO(...) log_print(LOG_LEVEL_INFO, LOG_TAG, ##__VA_ARGS__)
#define LOG_DEBUG(...) log_print(LOG_LEVEL_DEBUG, LOG_TAG, ##__VA_ARGS__)

void log_print(int log_level, char *tag, const char *fmt, ...); 

#endif
