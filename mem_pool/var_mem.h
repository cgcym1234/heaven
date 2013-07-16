#ifndef _MEM_H
#define _MEM_H

#ifdef __cplusplus
extern "C"{
#endif

//#include <stddef.h>

typedef unsigned int int_t;
typedef struct vpool_s vpool_t;

vpool_t *mem_init(int_t initsize);

void mem_destroy(vpool_t *vpool);

void *mem_alloc(vpool_t *vpool, int_t addr_size);

void mem_free(vpool_t *vpool, void *p);

#ifdef __cplusplus
}
#endif

#endif

