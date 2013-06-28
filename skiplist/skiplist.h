#ifndef _SKIP_LIST_H_
#define _SKIP_LIST_H_

#ifdef __cplusplus
extern "C"{
#endif

#define SKIPLIST_MAX_LEVEL 32

typedef int my_key_t;
typedef int value_t;
typedef struct skip_node_s skip_node_t;

struct skip_node_s
{
    my_key_t key;
    value_t value;
    int top_level;
    skip_node_t *prev;
    skip_node_t *level_next[];
};

typedef struct
{
    int cur_level;
    unsigned long length;
    skip_node_t *head;
    skip_node_t *tail;
    skip_node_t *recycle;
}skip_list_t;

skip_list_t *skip_init();
void skip_destroy(skip_list_t *slist);

int skip_insert(skip_list_t *slist, my_key_t key, value_t value);
int skip_delte(skip_list_t *slist, my_key_t key);
skip_node_t *skip_find(skip_list_t *slist, my_key_t key);

void skip_info(skip_list_t *slist);
void skip_reverse_info(skip_list_t *slist);

#ifdef __cplusplus
}
#endif

#endif

