#ifndef _FIX_MEM_H_
#define _FIX_MEM_H_

#ifdef __cplusplus
extern "C"{
#endif

typedef struct fix_mpool_s fix_mpool_t;

fix_mpool_t *fmem_create(int init_size, int data_size);
void fmem_destroy(fix_mpool_t *pool);

void *fmem_alloc(fix_mpool_t *pool);
void fmem_free(fix_mpool_t *pool, void *p);

void fmem_info(fix_mpool_t *pool);

#ifdef __cplusplus
}
#endif

#endif

