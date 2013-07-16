#include "thread_pool.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

FILE *fp;

void tvsub(struct timeval *out, struct timeval *in)
{
        if ( (out->tv_usec -= in->tv_usec) < 0) {
                --out->tv_sec;
                out->tv_usec += 1000000;
        }
        out->tv_sec -= in->tv_sec;
}

int starttime(struct timeval * ptv_start)
{
        return(gettimeofday(ptv_start, NULL));
}

unsigned long stoptime(struct timeval tv_start)
{
        unsigned long  clockus;
        struct timeval tv_stop;
        if (gettimeofday(&tv_stop, NULL) == -1)
                return 0;
        tvsub(&tv_stop, &tv_start);
        clockus = tv_stop.tv_sec * 1000000 + tv_stop.tv_usec;
        return(clockus); //return unit "us"
}


void test_fun(void *arg)
{
    int i = (int)arg;
    fprintf(fp, "%d\n", i);
    fflush(fp);

    return;
}

int main()
{
    fp = fopen("log.txt","w+");

    long long int cost = 0;
    struct timeval tv;
    starttime(&tv);

    thread_pool_t *pool = threadpool_create(2, 4, 10000);
    int i;
    for(i = 0; i < 1000; i++)
    {
        dispatch(pool, test_fun, (void *)i);
    }
    //sleep(5);
    dispatch(pool, test_fun, (void *)i, EMG_PRI);
    threadpool_destroy(pool, 1);

    cost = stoptime(tv);
    printf("%lld\n", cost/1000);

    return 0;
}

