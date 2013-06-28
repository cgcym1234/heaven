#ifndef _MEM_H
#define _MEM_H

#ifdef __cplusplus
extern "C"{
#endif

typedef unsigned int int_t;
typedef struct mpool_s mpool_t;

mpool_t *mem_init(int_t initsize, int_t datasize);

void mem_destroy(mpool_t *pool);

void *mem_alloc(mpool_t *pool);

void mem_free(mpool_t *pool, void *p);

void mem_info(mpool_t *pool);

#ifdef __cplusplus
}
#endif

#endif

