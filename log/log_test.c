#include "log.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

int main()
{
    if(log_init(LOG_DEBUG, "./log", "test", 1) == -1) return 0;
    log_write("log_write ok, log path:[%s], log level:[%d]",
              "test", LOG_DEBUG);
    log_error("log_error ok, log path:[%s], log level:[%d]",
              "test", LOG_DEBUG);
    log_warning("log_warning ok, log path:[%s], log level:[%d]",
              "test", LOG_DEBUG);
    log_info("log_info ok, log path:[%s], log level:[%d]",
              "test", LOG_DEBUG);
    log_debug("log_debug ok, log path:[%s], log level:[%d]",
              "test", LOG_DEBUG);
    int i;
    for(i = 0; i < 100000; i++) log_debug("log test:%d", i);
    log_close();

    return 0;
}


