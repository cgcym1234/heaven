#include<stdlib.h>
#include<string.h>
#include<stdio.h>

#include "fix_mem.h"

#define MEM_ERR -1
#define ALIGN 8
static int_t REAPPLY_SIZE = 40;

#define ROUND_UP(bytes) (((bytes) + ALIGN-1)&~(ALIGN - 1))

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

struct mpool_s{
    int_t total;
    int_t avail;
    int_t data_size;
    addr_t *head;
    chunk_t *data;
};

static int chunk_alloc(mpool_t *pool, int_t apply_size)
{
    chunk_t *chunk = malloc(sizeof(chunk_t));
    chunk->p = calloc(apply_size, pool->data_size);
    if(chunk->p == NULL) return MEM_ERR;

    int i;
    addr_t *temp;
    for (i = 0; i < apply_size; i++)
    {
        temp = (addr_t *)(chunk->p + i * pool->data_size);
        temp->next = pool->head;
        pool->head = temp;
    }
    chunk->start = (char *)chunk->p;
    chunk->end = chunk->start + apply_size* pool->data_size;

    chunk->next = pool->data;
    pool->data = chunk;

    return 0;
}

mpool_t *mem_init(int_t initsize, int_t datasize)
{
    if (!initsize || !datasize) return NULL;
    mpool_t *pool = malloc(sizeof(mpool_t));
    pool->total = initsize;
    pool->data_size = ROUND_UP(datasize);
    pool->avail = initsize;
    pool->head = NULL;
    pool->data = NULL;

    if(chunk_alloc(pool, initsize) == MEM_ERR)
    {
        fprintf(stderr,"Out Of Memory!!!");
        abort();
    }

    return pool;
}
void mem_destroy(mpool_t *pool)
{
    while (pool->data)
    {
        chunk_t *temp = pool->data->next;
        free(pool->data->p);
        free(pool->data);
        pool->data = temp;
    }
    free(pool);
}
void *mem_alloc(mpool_t *pool)
{
    if (pool->head)
    {
        void *p = (void *)pool->head;
        pool->head = pool->head->next;
        pool->avail--;
        return p;
    }
    else
    {
        if(chunk_alloc(pool, REAPPLY_SIZE) == MEM_ERR)
            return NULL;

        pool->total += REAPPLY_SIZE;
        pool->avail += REAPPLY_SIZE;
        REAPPLY_SIZE *= 2;

        return mem_alloc(pool);
    }
}
void mem_free(mpool_t *pool, void *p)
{
    if(!p) return;
    addr_t *temp = (addr_t *)p;
    temp->next = pool->head;
    pool->head = temp;
    pool->avail++;

    return;
}
void mem_info(mpool_t *pool)
{
    //todo?
}

