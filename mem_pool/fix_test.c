#include "fix_mem.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <malloc.h>

#define TIMES 2000000

typedef struct
{
    int i;
    char c;
}test;

test *p[TIMES];

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

void test1()
{
    fix_mpool_t *pool = fmem_create(TIMES, sizeof(test));
    int i;
    long long int cost = 0;
    struct timeval tv;
    starttime(&tv);
    //pool_t *pool = mem_init(TIMES, sizeof(test));

    for(i = 0; i < TIMES; i++)
    {
        p[i] = (test *)fmem_alloc(pool);
        //memset(p[i], 0, sizeof(test));
    }
    for(i = 0; i < TIMES; i++)
    {
        fmem_free(pool, p[i]);
    }
    // mem_info(pool);
    cost = stoptime(tv);
    fmem_destroy(pool);
    printf("%lld\n", cost/1000);

}

void test2()
{
    int i;
    long long int cost = 0;
    struct timeval tv;
    starttime(&tv);

    for(i = 0; i < TIMES; i++)
    {
        p[i] = (test *)malloc(sizeof(test));

    }
    //cost = stoptime(tv);
    for(i = 0; i < TIMES; i++)
    {
        free(p[i]);
    }
    cost = stoptime(tv);
    printf("%lld\n", cost/1000);
}

int main()
{
    test1();
    test2();
    return 0;
}


