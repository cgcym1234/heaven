#include<stdlib.h>
#include<string.h>
#include<stdio.h>

#include "stl_mem.h"

#define ALIGN 8
#define MAX_BYTES 256
#define LIST_SIZE (MAX_BYTES/ALIGN)
#define REAPPLY_SIZE 40

#define ROUND_UP(bytes) (((bytes) + ALIGN-1)&~(ALIGN-1))
#define LIST_INDEX(bytes) (((bytes) + ALIGN-1)/ALIGN - 1)

typedef struct chunk_s chunk_t;
typedef struct addr_s addr_t;

struct chunk_s{
    void *p;
    char *start;
    char *end;
    chunk_t *next;
};

struct addr_s{
    addr_t *next;
};

struct vpool_s{
    char *start;
    char *end;
    addr_t *free[LIST_SIZE];
    chunk_t *data;
};

static void *chunk_alloc(vpool_t *vpool, int_t size, int *n)
{
    char *result;
    int_t total_bytes = *n * size;
    int_t heap_left = vpool->end - vpool->start;
    addr_t **my_free, *p;
    if(heap_left >= total_bytes)
    {
        result= vpool->start;
        vpool->start += total_bytes;
        return result;
    }
    else if(heap_left >= size)
    {
        result= vpool->start;
        *n = heap_left/size;
        vpool->start += *n * size;
        return result;
    }
    else
    {
        if (heap_left > 0)
        {
            my_free = vpool->free + LIST_INDEX(heap_left);
            p = (addr_t *)vpool->start;
            p->next = *my_free;
            (*my_free)->next = p;
        }
        int_t heap_to_get=2 * total_bytes;
        chunk_t *chunk = malloc(sizeof(chunk_t));
        chunk->p = malloc(heap_to_get);
        if(chunk->p == NULL)
        {
            fprintf(stderr,"Out Of Memory!!!");
            abort();
        }
        chunk->start = (char *)chunk->p;
        chunk->end = chunk->start + heap_to_get;

        vpool->start = chunk->start;
        vpool->end = chunk->end;

        chunk->next = vpool->data;
        vpool->data = chunk;

        return chunk_alloc(vpool, size, n);
    }
}
static void refill(vpool_t *vpool, int_t size)
{
    int nobj = REAPPLY_SIZE;
    char *chunk = (char *)chunk_alloc(vpool, size, &nobj);
    addr_t **my_free;
/*
    addr_t *tmp;
    my_free =  vpool->free + LIST_INDEX(size);
    int i;
    for(i = 0; i < nobj; i++)
    {
        tmp = (addr_t *)(chunk + i*size);
        tmp->next = *my_free;
        *my_free = tmp;
    }*/

    addr_t *cur, *next;
    my_free =  vpool->free + LIST_INDEX(size);
    cur = *my_free = (addr_t*)chunk;
    int i;
    for(i = 1; ; i++)
    {

        if(i == nobj)
        {
            cur->next = NULL;
            break;
        }
        next = (addr_t *)((char *)cur + size);
        cur->next = next;
        cur = next;
    }

}
vpool_t *mem_init(int_t initsize)
{
    if (!initsize) return NULL;
    vpool_t *vpool = malloc(sizeof(vpool_t));
    int_t bytes = ROUND_UP(initsize);
    //vpool->cur = 0;
    vpool->data = NULL;
    memset(vpool->free, 0, sizeof(vpool->free));
    chunk_t *chunk = malloc(sizeof(chunk_t));
    chunk->p = malloc(bytes);
    if(chunk->p == NULL)
    {
        fprintf(stderr,"Out Of Memory!!!");
        abort();
    }
    chunk->start = (char *)chunk->p;
    chunk->end = chunk->start + bytes;

    vpool->start = chunk->start;
    vpool->end = chunk->end;

    chunk->next = vpool->data;
    vpool->data = chunk;

    return vpool;
}
void mem_destroy(vpool_t *vpool)
{
    while (vpool->data)
    {
        chunk_t *temp = vpool->data->next;
        free(vpool->data->p);
        free(vpool->data);
        vpool->data = temp;
    }
    free(vpool);
}
void *mem_alloc(vpool_t *vpool, int_t addr_size)
{
    if(addr_size > (int_t)MAX_BYTES)
    {
        fprintf(stderr,"Does not support the application of the size!!!");
        abort();
    }
    addr_t **my_free;
    addr_t *result;
    my_free =  vpool->free + LIST_INDEX(addr_size);
    result = *my_free;
    if(result == 0)
    {
        refill(vpool, ROUND_UP(addr_size));
        return mem_alloc(vpool, addr_size);
    }

    *my_free = result->next;
    return result;
}
void mem_free(vpool_t *vpool, void *p, int_t addr_size)
{
    if (!p) return;
    if(addr_size > (int_t)MAX_BYTES)
    {
        return;
    }
    //memset(p, 0, addr_size);
    addr_t *temp = (addr_t *)p;
    addr_t **my_free = vpool->free + LIST_INDEX(addr_size);
    temp->next = *my_free;
    *my_free= temp;
}

