#include<stdlib.h>
#include<string.h>
#include<stdio.h>

#include "fix_mem.h"

typedef unsigned int int_t;
typedef struct chunk_s chunk_t;
typedef struct block_s block_t;

struct chunk_s{
    void *p;
    char *start;
    char *end;
    chunk_t *next;
};

struct block_s{
    block_t *next;
};

struct fix_mpool_s{
    int_t total;
    int_t avail;
    int_t block_size;
    block_t *head;
    chunk_t *data;
};

#define MEM_ERR -1
#define ALIGN 8
#define add_node_to_head(node, head) {(node)->next = head;(head) = (node);}
#define round_up(bytes) (((bytes) + ALIGN-1)&~(ALIGN - 1))
#define exit_with_message(msg)     do{ \
        fprintf(stderr, msg);          \
        exit(1);                       \
        }while(0)
#define exit_if_fail_with_message(expr, msg)   do{ \
        if(expr) { } else{                         \
            fprintf(stderr, msg);                  \
            exit(1);                               \
        }; }while(0)

static int_t reapply_size = 80;

static int chunk_alloc(fix_mpool_t *pool, int_t apply_size)
{
    chunk_t *chunk = malloc(sizeof(chunk_t));
    if(chunk == NULL) return MEM_ERR;
    chunk->p = calloc(apply_size, pool->block_size);
    if(chunk->p == NULL) return MEM_ERR;

    int i;
    block_t *temp;
    for(i = 0; i < apply_size; i++)
    {
        temp = (block_t *)(chunk->p + i * pool->block_size);
        add_node_to_head(temp, pool->head);
    }

    chunk->start = (char *)chunk->p;
    chunk->end = chunk->start + apply_size * pool->block_size;
    add_node_to_head(chunk, pool->data);

    return 0;
}

fix_mpool_t *fmem_create(int init_nums, int datasize)
{
    if(init_nums <= 0 || datasize <= 0) return NULL;

    init_nums = round_up(init_nums);
    fix_mpool_t *pool = malloc(sizeof(fix_mpool_t));
    exit_if_fail_with_message(pool, "fmem_create() failed,Out Of Memory!!!\n");

    pool->total = init_nums;
    pool->block_size = round_up(datasize);
    pool->avail = init_nums;
    pool->head = NULL;
    pool->data = NULL;

    if(chunk_alloc(pool, init_nums) == MEM_ERR)
    exit_with_message("chunk_alloc() failed, Out Of Memory!!!\n");

    return pool;
}

void fmem_destroy(fix_mpool_t *pool)
{
    if(!pool) return;

    chunk_t *temp;
    while(pool->data)
    {
        temp = pool->data;
        pool->data = pool->data->next;
        free(temp->p);
        free(temp);
    }
    free(pool);
}

void *fmem_alloc(fix_mpool_t *pool)
{
    if(pool->head)
    {
        void *p = (void *)pool->head;
        pool->head = pool->head->next;
        pool->avail--;
        return p;
    }
    else
    {
        if(chunk_alloc(pool, reapply_size) == MEM_ERR) return NULL;

        pool->total += reapply_size;
        pool->avail += reapply_size;
        reapply_size *= 2;

        return fmem_alloc(pool);
    }
}

void fmem_free(fix_mpool_t *pool, void *p)
{
    if(!p) return;
    block_t *temp = (block_t *)p;
    add_node_to_head(temp, pool->head);
    pool->avail++;
}

void fmem_info(fix_mpool_t *pool)
{
    if(!pool) return;

    printf("total memory nums:%u\ncurrent avails:%u\ndata size:%u\n",
           pool->total, pool->avail, pool->block_size);
}

