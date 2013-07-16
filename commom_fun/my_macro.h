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

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

#define bit_swap32(_x) ( \
        ((0xAAAAAAAA & (unsigned int)(_x)) >> 1) | \
        ((0x55555555 & (unsigned int)(_x)) << 1) )

#define add_node_to_head(node, head) { (node)->next = (head);(head) = (node); }
#define add_node_to_tail(node, tail) { (tail)->next = (node);(tail) = (node); }

#define ALIGN 8
#define round_up(bytes) (((bytes) + ALIGN-1)&~(ALIGN - 1))

#define get_max_value_of_int(x) { (x) = (1 << 31) - 1; }
#define get_min_value_of_int(x) { (x) = 1 << 31; }
#define swap(a, b) { typeof(a) c = a; a = b; b = c; }
#define int_swap(a, b) { (a)^=(b)^=(a)^=(b); }
#define get_nbit_value(m, n) ((m >> (n-1)) & 1)
#define set_nbit_to_1(m, n) (m | (1 << n-1))
#define set_nbit_to_0(m, n) (m & ~(1 << n-1))


#define return_if_fail(expr)                   do{ \
    if(expr) { } else{                             \
        fprintf(stderr,                            \
        "file %s: line %d: assertion '%s' failed\n", \
        __FILE__, __LINE__, #expr);                \
        return;                                    \
    }; }while(0)

#define return_val_if_fail(expr, val)          do{ \
    if(expr) { } else{                             \
        fprintf(stderr,                            \
        "file %s: line %d: assertion '%s' failed\n", \
        __FILE__, __LINE__, #expr);                \
        return(val);                               \
    }; }while(0)

#define return_if_fail_with_message(expr, msg) do{ \
    if(expr) { } else{                             \
        fprintf(stderr, msg"\n");                  \
        return;                                    \
    }; }while(0)

#define return_val_if_fail_with_message(expr, val, msg) do{ \
    if(expr) { } else{                             \
        fprintf(stderr, msg"\n");                  \
        return(val);                               \
    }; }while(0)

#define exit_if_fail(expr)                     do{ \
    if(expr) { } else{                             \
        fprintf(stderr,                            \
        "file %s: line %d: assertion '%s' failed\n", \
        __FILE__, __LINE__, #expr);                \
        exit(1);                                   \
    }; }while(0)

#define exit_if_fail_with_message(expr, msg)   do{ \
    if(expr) { } else{                             \
        fprintf(stderr, msg"\n");                  \
        exit(1);                                   \
    }; }while(0)

#define exit_with_message(msg)                 do{ \
        fprintf(stderr, msg"\n");                  \
        exit(1);                                   \
    }while(0)

#endif

