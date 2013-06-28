#include "skiplist.h"

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


void test_fun(int num)
{
    long long int cost = 0;
    struct timeval tv;
    starttime(&tv);

    skip_list_t *sl = skip_init();
    int i = 0;
    for(; i < num; i++) skip_insert(sl, i, 2*i);

    for(i = num - 1; i > 0; i--)
    if(!skip_find(sl, i)) printf("skip_find() failed key = %d\n", i);

    for(i = 0; i < num; i++)
    if(skip_delte(sl, i) == -1) printf("skip_delte() failed key = %d\n", i);

    skip_destroy(sl);

    cost = stoptime(tv);
    printf("cost: %lld ms\n", cost/1000);

    return;
}

int main()
{
    long long int cost = 0;
    struct timeval tv;
    starttime(&tv);

    skip_list_t *sl = skip_init();
    int i = 0;
    for(; i < 30; i++) skip_insert(sl, i, 2*i);
    skip_info(sl);
    skip_reverse_info(sl);

    int key = 10;
    skip_node_t *tt = skip_find(sl, key);
    if(tt) printf("skip_find() key = %d,value = %d\n", key, tt->value);

    key = 22;
    if(skip_delte(sl, key) != -1) printf("skip_delte() key = %d\n", key);
    key = 1;
    if(skip_delte(sl, key) != -1) printf("skip_delte() key = %d\n", key);

    for(; i < 60; i++) skip_insert(sl, i, 2*i);
    skip_info(sl);
    skip_reverse_info(sl);
    skip_destroy(sl);

    cost = stoptime(tv);
    printf("%lld\n", cost/1000);

    int num = 100;
    test_fun(num);

    return 0;
}

