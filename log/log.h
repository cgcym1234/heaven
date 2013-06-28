#ifndef _LOG_H_
#define _LOG_H_

#include <stdarg.h>

typedef enum
{
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG,
    LOG_WRITE
}log_level_t;

int log_init(int log_level, const char *log_path, const char *log_name, int use_thread);
void log_it(log_level_t level, const char *fmt, ...);
void log_set_level(int level);
void log_close();

#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
    #define log_error(...) \
        log_it(LOG_ERROR, __VA_ARGS__)
    #define log_warning(...) \
        log_it(LOG_WARNING, __VA_ARGS__)
    #define log_info(...) \
        log_it(LOG_INFO, __VA_ARGS__)
    #define log_debug(...) \
        log_it(LOG_DEBUG, __VA_ARGS__)
    #define log_write(...) \
        log_it(LOG_WRITE, __VA_ARGS__)

#elif defined __GNUC__
    #define log_error(fmt, args...) \
        log_it(LOG_ERROR, fmt, ## args)
    #define log_warning(fmt, args...) \
        log_it(LOG_WARNING, fmt, ## args)
    #define log_info(fmt, args...) \
        log_it(LOG_INFO, fmt, ## args)
    #define log_debug(fmt, args...) \
        log_it(LOG_DEBUG, fmt, ## args)
    #define log_write(fmt, args...) \
        log_it(LOG_WRITE, fmt, ## args)
#endif

#endif

