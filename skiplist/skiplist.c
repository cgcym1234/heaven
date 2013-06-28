#include "skiplist.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SKIPLIST_SIZE_OF_EACH_APPLICATION 40

#define exit_if_null_with_message(p, m) { if(!(p)) \
        {fprintf(stderr, m); exit(1);} }

#define add_node_to_head(node, head) { (node)->level_next[0] = (head);(head) = (node); }

static inline int skip_choose_level();
static inline skip_node_t *skip_node_create(int level, int key, int value);
static void recycle_node_create(skip_list_t *slist);
static int get_recycle_node(skip_list_t *slist, skip_node_t **node);
static void delete_node(skip_list_t *slist, skip_node_t *bingo, skip_node_t **update);

static inline skip_node_t *skip_node_create(int level, int key, int value)
{
    skip_node_t *ns = (skip_node_t *)malloc(sizeof(skip_node_t) + level*sizeof(skip_node_t *));
    ns->key = key;
    ns->value = value;
    ns->top_level = level;

    return ns;
}

static void recycle_node_create(skip_list_t *slist)
{
    skip_node_t *node;
    int level;
    int i;
    for(i = 0; i < SKIPLIST_SIZE_OF_EACH_APPLICATION; i++)
    {
        level = skip_choose_level();
        node = skip_node_create(level, 0, 0);
        add_node_to_head(node, slist->recycle);
    }

    return;
}

static int get_recycle_node(skip_list_t *slist, skip_node_t **node)
{
    if(slist->recycle)
    {
        *node = slist->recycle;
        slist->recycle = slist->recycle->level_next[0];
    }
    else
    {
        recycle_node_create(slist);
        *node = slist->recycle;
        slist->recycle = slist->recycle->level_next[0];
    }

    return 0;
}

static void delete_node(skip_list_t *slist, skip_node_t *bingo, skip_node_t **update)
{
    int i;
    for(i = 0; i < slist->cur_level; i++)
    {
        if(update[i]->level_next[i] == bingo)
        {
            update[i]->level_next[i] = bingo->level_next[i];
        }
    }

    if(bingo->level_next[0]) bingo->level_next[0]->prev = bingo->prev;
    else slist->tail = bingo->prev;

    for(i = slist->cur_level - 1; slist->cur_level > 1 && i >= 0; i--)
    {
        /*delete the level which noly has the head node*/
        if(!slist->head->level_next[i]) slist->cur_level--;
        else break;
    }

    slist->length--;

    return;
 }

static inline int skip_choose_level()
{
    int level = 1;
    while((rand()&0xFFFF) < (0.5 * 0xFFFF)) level += 1;
    return (level<SKIPLIST_MAX_LEVEL) ? level : SKIPLIST_MAX_LEVEL;
}

skip_list_t *skip_init()
{
    skip_list_t *slist = (skip_list_t *)malloc(sizeof(skip_list_t));
    exit_if_null_with_message(slist, "skip_init():calloc() failed\n");

    slist->cur_level = 1;
    slist->length = 0;

    slist->head = skip_node_create(SKIPLIST_MAX_LEVEL, -1, 0);
    int i;
    for(i = 0; i < SKIPLIST_MAX_LEVEL; i++) slist->head->level_next[i] = NULL;

    slist->head->prev = NULL;
    slist->tail = NULL;
    slist->recycle = NULL;
    srand((unsigned int)time(NULL));
    recycle_node_create(slist);

    return slist;
}

void skip_destroy(skip_list_t *slist)
{
    if(!slist) return;
    if(!slist->head) return;

    skip_node_t *node;
    while(slist->head)
    {
        node = slist->head;
        slist->head = slist->head->level_next[0];
        free(node);
    }
    while(slist->recycle)
    {
        node = slist->recycle;
        slist->recycle = slist->recycle->level_next[0];
        free(node);
    }
    free(slist);

    return;
}

int skip_insert(skip_list_t *slist, my_key_t key, value_t value)
{
    skip_node_t *node = slist->head;
    skip_node_t *update[SKIPLIST_MAX_LEVEL];

    /*find the proper node of all level, save*/
    int i = slist->cur_level - 1;
    for(; i >= 0; i--)
    {
        while(node->level_next[i] && node->level_next[i]->key < key)
            node = node->level_next[i];
        update[i] = node;
    }
    /*make sure there is no duplicate key*/
    if(node && node->key == key) return -1;

    skip_node_t *new_node;
    get_recycle_node(slist, &new_node);
    new_node->key = key;
    new_node->value = value;

    int new_level = new_node->top_level;
    /*if new level is higher, adjust the list*/
    if(new_level > slist->cur_level)
    {
        for(i = slist->cur_level; i < new_level; i++) update[i] = slist->head;
        slist->cur_level = new_level;
    }

    /*insert new node*/
    for(i = 0; i < new_level; i++)
    {
        new_node->level_next[i] = update[i]->level_next[i];
        update[i]->level_next[i] = new_node;
    }
    slist->length++;

    new_node->prev = (update[0] == slist->head ? NULL : update[0]);
    if(new_node->level_next[0]) new_node->level_next[0]->prev = new_node;
    else slist->tail = new_node;

    return 0;
}

int skip_delte(skip_list_t *slist, my_key_t key)
{
    skip_node_t *node = slist->head;
    skip_node_t *update[SKIPLIST_MAX_LEVEL];

    int i = slist->cur_level - 1;
    for(; i >= 0; i--)
    {
        while(node->level_next[i] && node->level_next[i]->key < key)
            node = node->level_next[i];
        update[i] = node;
    }

    node = node->level_next[0];
    if(node && node->key == key)
    {
        delete_node(slist, node, update);
        add_node_to_head(node, slist->recycle);
    }

    return 0;
}

skip_node_t *skip_find(skip_list_t *slist, my_key_t key)
{
    skip_node_t *node = slist->head;

    int i = slist->cur_level - 1;
    for(; i >= 0; i--)
    {
        while(node->level_next[i] && node->level_next[i]->key < key)
            node = node->level_next[i];
        if(node->level_next[i] && node->level_next[i]->key == key)
            return node->level_next[i];
    }

    return NULL;
}

void skip_info(skip_list_t *slist)
{
    if(!slist) return;

    skip_node_t *node;
    int i;
    for(i = slist->cur_level - 1; i >= 0; --i)
    {
        printf("level[%d]: ", i + 1);
        node = slist->head;
        while(node)
        {
            printf("%d -> ", node->key);
            node = node->level_next[i];
            //if(node) printf(" -> ");
        }
        printf("NULL\n");
    }
}

void skip_reverse_info(skip_list_t *slist)
{
    if(!slist || !slist->tail) return;

    skip_node_t *node = slist->tail;
    printf("reverse node:\n");
    while(node)
    {
        printf("%d -> ", node->key);
        node = node->prev;
    }
    printf("NULL\n");
}

