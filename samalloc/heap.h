#ifndef _HEAP_H_
#define _HEAP_H_

#include "config.h"

typedef struct free_block_header *header_p;
struct free_block_header
{
    size_t size;
    header_p next;
    header_p prev;
};

#define HEADER_SIZE (sizeof(struct free_block_header))

static __inline__ size_t block_size(header_p h)
{
    return h->size & (~1);
}

static __inline__ int is_free(header_p h)
{
    return h->size & 1;
}

static __inline__ void set_free(header_p h)
{
    h->size |= 1;
}

static __inline__ void set_used(header_p h)
{
    h->size &= (~1);
}

static __inline__ void *payload(header_p h)
{
    return ((void*) h) + HEADER_SIZE;
}

static __inline__ void insert_block(header_p *heap, header_p block, header_p prev, header_p next)
{
    block->next = next;
    block->prev = prev;
    if (prev)
        prev->next = block;
    else
        *heap = block;
    if (next)
        next->prev = block;
}

static __inline__ void delete_block(header_p *heap, header_p block, header_p prev, header_p next)
{
    if (next)
        next->prev = prev;
    if (prev)
        prev->next = next;
    else
        *heap = next;
}

static __inline__ header_p merge_blocks(header_p a, header_p b)
{
    if (a->next != b)
        return a;
    a->size += b->size + HEADER_SIZE;
    a->next = b->next;
    return a;
}

#endif

