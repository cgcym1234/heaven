#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <malloc.h>
#include <unistd.h>
#include <errno.h>

#include "log.h"
#include "thread_pool.h"

#define VALID_LEN 256
#define MSG_LEN 2048
#define MAX_LOG_SIZE 300*1024*1024

typedef enum
{
    LOG_TAG_ERROR,
    LOG_TAG_WARNING,
    LOG_TAG_INFO,
    LOG_TAG_DEBUG,
    LOG_TAG_WRITE
}log_tag_t;

typedef struct
{
    int cur_log_size;
    char log_name[VALID_LEN];
    char log_path[VALID_LEN];
    log_tag_t tag;
    pthread_mutex_t mtx;
    FILE *log_fp;
    thread_pool_t *tpool;
}log_handle_t;

typedef struct
{
	int level;
	char msg[MSG_LEN];
}log_arg_t;

static log_handle_t log_handle;
static char *level_string[] = {
    "ERROR",
    "WARN ",
    "INFO ",
    "DEBUG",
    "WRITE"
};
static const char *level_to_string(int level)
{
    switch(level)
    {
        case LOG_ERROR:
            return "ERROR";
        case LOG_WARNING:
            return "WARNING";
        case LOG_INFO:
            return "INFO";
        case LOG_DEBUG:
            return "DEBUG";
        case LOG_WRITE:
            return "WRITE";
        default:
            return "DEBUG";
    }

    return NULL;
}

static int my_mkdir(const char *dir_name)
{
    if(!dir_name) return -1;

    if(access(dir_name, F_OK) == 0) return 0;
    if(mkdir(dir_name, S_IRUSR | S_IWUSR | S_IXUSR) == 0) return 0;

    const char *begin = dir_name + 1;
    const char *end;
    char parent_dir[512] = {0};
    while((end = strchr(begin, '/')) != NULL)
    {
        begin = end + 1;
        memcpy(parent_dir, dir_name, end - dir_name);
        parent_dir[end - dir_name] = '\0';

        if(access(parent_dir, F_OK) == 0) continue;
        if(mkdir(parent_dir, S_IRUSR | S_IWUSR | S_IXUSR) < 0)
        {
            fprintf(stderr, "mkdir():[%s] failed when make parent_dir:[%s], errno:[%s]\n",
                    dir_name, parent_dir, strerror(errno));
            return -1;
        }
    }

    if(mkdir(dir_name, S_IRUSR | S_IWUSR | S_IXUSR) < 0)
    {
        fprintf(stderr, "mkdir():[%s] failed, errno:[%s]\n",
                dir_name, strerror(errno));
        return -1;
    }

    return 0;
}

static void get_cur_date(char *str)
{
    if(!str) return;

    struct timeval tv;
    struct tm ctm;
    gettimeofday(&tv, NULL);
    localtime_r(&(tv.tv_sec), &ctm);

    sprintf(str, "%04d-%02d-%02d_%02d:%02d:%02d", ctm.tm_year+1900,
            ctm.tm_mon+1, ctm.tm_mday, ctm.tm_hour, ctm.tm_min, ctm.tm_sec);

    return;
}

static FILE * create_new_log(const char *name, const char *path)
{
    char date[16] = {0};
    get_cur_date(date);
    char log_name[VALID_LEN * 2] = {0};
    snprintf(log_name, sizeof(log_name), "%s/%s_%d_%s.log",
             path, name, getpid(), date);
    FILE *fp;
    fp = fopen(log_name, "a+");
    if(fp == NULL)
    {
        fprintf(stderr, "create logfile:[%s] failed:[%s]\n", log_name, strerror(errno));
    }

    return fp;
}

static void write_log(log_level_t level ,const char *buf)
{
/*
    char buf[2048] = {0};
    vsnprintf(buf, sizeof(buf), fmt, ap);// this is safe
*/

    struct timeval tv;
    struct tm cur_time;
    gettimeofday(&tv, NULL);
    localtime_r(&(tv.tv_sec), &cur_time);

    pthread_mutex_lock(&log_handle.mtx);
    int write_len = 0;
    write_len = fprintf(log_handle.log_fp, "%02d-%02d-%02d %02d:%02d:%02d:%lu %s:%s\n",
                        cur_time.tm_year+1900, cur_time.tm_mon+1, cur_time.tm_mday,
                        cur_time.tm_hour, cur_time.tm_min, cur_time.tm_sec,
                        tv.tv_usec/1000, level_string[level], buf);
    fflush(log_handle.log_fp);
    if(write_len > 0)
    {
        log_handle.cur_log_size += write_len;
        if(log_handle.cur_log_size >= MAX_LOG_SIZE)
        {
            log_handle.cur_log_size = 0;
            if(log_handle.log_fp != stderr) fclose(log_handle.log_fp);
            log_handle.log_fp = create_new_log(log_handle.log_name, log_handle.log_path);
            if(log_handle.log_fp == NULL) log_handle.log_fp = stderr;
        }
    }
    pthread_mutex_unlock(&log_handle.mtx);

    return;
}

void thread_write_log(void *arg)
{
    log_arg_t *info = (log_arg_t *)arg;

    struct timeval tv;
    struct tm cur_time;
    gettimeofday(&tv, NULL);
    localtime_r(&(tv.tv_sec), &cur_time);

    int write_len = 0;
    write_len = fprintf(log_handle.log_fp, "%02d-%02d-%02d %02d:%02d:%02d:%lu %s:%s\n",
                        cur_time.tm_year+1900, cur_time.tm_mon+1, cur_time.tm_mday,
                        cur_time.tm_hour, cur_time.tm_min, cur_time.tm_sec,
                        tv.tv_usec/1000, level_string[info->level], info->msg);
    fflush(log_handle.log_fp);
    if(write_len > 0)
    {
        log_handle.cur_log_size += write_len;
        if(log_handle.cur_log_size >= MAX_LOG_SIZE)
        {
            log_handle.cur_log_size = 0;
            if(log_handle.log_fp != stderr) fclose(log_handle.log_fp);
            log_handle.log_fp = create_new_log(log_handle.log_name, log_handle.log_path);
            if(log_handle.log_fp == NULL) log_handle.log_fp = stderr;
        }
    }
    free(arg);

    return;
}

void log_it(log_level_t level, const char *fmt, ...)
{
    do
    {
        if(level == LOG_WRITE) break;

        switch(log_handle.tag)
        {
            case LOG_TAG_ERROR:
                if(level <= LOG_ERROR)
                    break;
                return;
            case LOG_TAG_WARNING:
                if(level <= LOG_WARNING)
                    break;
                return;
            case LOG_TAG_INFO:
                if(level <= LOG_INFO)
                    break;
                return;
            case LOG_TAG_DEBUG:
                if(level <= LOG_DEBUG)
                    break;
                return;
            default:
                return;
        }
    }while(0);

    char buf[MSG_LEN] = {0};

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if(log_handle.tpool)
    {
        log_arg_t *arg = malloc(sizeof(log_arg_t));
        arg->level = level;
        memcpy(arg->msg, buf, MSG_LEN);
        dispatch(log_handle.tpool, thread_write_log, (void *)arg);

        return;
    }
    write_log(level, buf);

    return;
}

int log_init(int level, const char *path, const char *name, int use_thread)
{
    if(!path || !name)
    {
        fprintf(stderr, "log_init failed, path or name null\n");
        return -1;
    }
    if(level < LOG_ERROR || level > LOG_DEBUG)
    {
        fprintf(stderr, "log_init failed, error level:[%d]\n", level);
        return -1;
    }
    if(access(path, F_OK) < 0 && my_mkdir(path) < 0)
    {
        fprintf(stderr, "create path:[%s] failed:[%s]\n", path, strerror(errno));
        return -1;
    }

    log_handle.cur_log_size = 0;
    strncpy(log_handle.log_name, name, VALID_LEN);
    strncpy(log_handle.log_path, path, VALID_LEN);
    log_handle.tag = level;
    pthread_mutex_init(&log_handle.mtx, NULL);
    log_handle.log_fp = create_new_log(name, path);
    if(log_handle.log_fp == NULL) return -1;

    log_handle.tpool = NULL;
    if(use_thread) log_handle.tpool = threadpool_create(1, 1, 500000);

    printf("log init ok, log_path:[%s], log_level:[%d]\n", path, level);

    return 0;
}

void log_set_level(int level)
{
    if(level < LOG_ERROR || level > LOG_DEBUG)
    {
        fprintf(stderr, "log_set_level failed, error level:[%d]\n", level);
        return;
    }
    log_handle.tag = level;

    return;
}

void log_close()
{
    if(log_handle.tpool) threadpool_destroy(log_handle.tpool, 1);
    if(log_handle.log_fp) fclose(log_handle.log_fp);
    pthread_mutex_destroy(&log_handle.mtx);
    printf("log close ok\n");

    return;
}

