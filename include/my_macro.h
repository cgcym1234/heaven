#ifndef _MY_MACRO_H_
#define _MY_MACRO_H_

#define LEN_32      32
#define LEN_64      64
#define LEN_128     128
#define LEN_256     256
#define LEN_512     512
#define LEN_1024    1024
#define LEN_2048    2048
#define LEN_4096    4096
#define LEN_10240   10240

//offset of struct member
#define my_offsetof(struct_type, member) ((size_t)(char *)&(((struct_type *)0)->member))

#define swap(a, b) { typeof(a) c = a; a = b; b = c; }
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

#define bit_swap32(_x) ( \
        ((0xAAAAAAAA & (unsigned int)(_x)) >> 1) | \
        ((0x55555555 & (unsigned int)(_x)) << 1) )

#define exit_if_null_with_message(p, m) { if((p) == NULL) \
        {fprintf(stderr, m); exit(1);} }

#define add_node_to_head(node, head) { (node)->next = (head);(head) = (node); }
#define add_node_to_tail(node, tail) { (tail)->next = (node);(tail) = (node); }

#define ALIGN 8
#define round_up(bytes) (((bytes) + ALIGN-1)&~(ALIGN - 1))

#endif
