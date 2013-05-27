#include "../include/compact.h"
#include <sys/mman.h>
extern "C" {
#include "morecore.h"
}

#include<string.h>

typedef struct item *item_p;
struct item{
    void* data;
    item_p next;
    item_p prev;
    size_t size;
};

static __inline__ size_t block_size(item *h)
{
    return h->size & (~1);
}

static __inline__ int is_free(item* h)
{
    return h->size & 1;
}

static __inline__ void set_free(item* h)
{
    h->size |= 1;
}

static __inline__ void set_used(item* h)
{
    h->size &= (~1);
}

item_p unmatched = NULL;
item_p matched = NULL;

int arena = 0;
int freemem = 0;
int usdmem = 0;
int freeblks = 0;
int usdblks = 0;

void new_page() {
    void* page = mmap(0, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    for (item_p i = (item_p) page; i < page + PAGE_SIZE; ++i) {
        i->next = unmatched;
        unmatched = i;
    }
    arena += PAGE_SIZE;
}

item_p get_unmatched_item() {
    if (unmatched == NULL)
        new_page();
    item_p res = unmatched;
    if (res == NULL)
        return NULL;
    unmatched = unmatched->next;
    res->next = NULL;
    return res;
}

void split_item(item_p i, size_t size) {
    if (block_size(i) <= size)
        return;
    item_p j = get_unmatched_item();
    if (j == NULL)
        return;
    j->size = block_size(i) - size;
    i->size = size;
    set_free(i); set_free(j);
    j->data = i->data + size;
    j->next = i->next;
    i->next = j;
    freeblks++;
}

void* get_block(size_t size) {
    item_p i = matched;
    item_p* last = &matched;
    while (i != NULL) {
        if (is_free(i) && block_size(i) >= size) {
            split_item(i, size);
            set_used(i);
            freeblks--;
            usdblks++;
            freemem -= block_size(i);
            usdmem += block_size(i);
            return (void*)i;
        }
        last = &(i->next);
        i = i->next;
    }
    i = get_unmatched_item();
    if (i == NULL)
        return NULL;
    size_t page_size = page_align(size);
    void* data = morecore(page_size);
    if (data == NULL) {
        i->next = unmatched;
        unmatched = i;
        return NULL;
    }
    arena += page_size;
    freemem += page_size;
    usdblks++;
    *last = i;
    i->data = data;
    i->size = page_size;
    split_item(i, size);
    set_used(i);
    freemem -= block_size(i);
    usdmem += block_size(i);
    return (void*)i;
}

struct myinfo myinfo() {
    struct myinfo res ={arena, freemem, usdmem, freeblks, usdblks,0};
    item_p i = matched;
    size_t max_free = 0;
    while (i != NULL) {
        size_t curr = block_size(i);
        if (is_free(i) && curr > max_free)
            max_free = curr;
        i = i->next;
    }
    res.maxfreeblk = max_free;
    return res;
}
void print_heap_dump() {
    printf("========= HEAP DUMP =========\n");
    item_p i = matched;
    while (i != NULL) {
        printf("%p %c %7db\n", i->data, is_free(i) ? 'f' : 'u', i->size & (~1));
        i = i->next;
    }
    printf("=============================\n");
}

void compact() {
    void* threshold = NULL;
    void* last = NULL;
    item_p i = matched;
    item_p* prev = &matched;
    while (i != NULL) {
        threshold = i->data + block_size(i);
        if (is_free(i)) {
            *prev = i->next;
            i->next = unmatched;
            unmatched = i;
            if (last == NULL)
                last = i->data;
            i = *prev;
        } else {
            if (last != NULL) {
                memcpy(last, i->data, block_size(i));
                i->data = last;
                last = last + block_size(i);
                prev = &(i->next);
            }
            i = i->next;
        }
    }
    freeblks = 0;
    if (last != NULL && last != threshold) {
        i = get_unmatched_item();
        i->data = last;
        i->size = (char*)threshold - (char*)last;
        set_free(i);
        *prev = i;
        freeblks++;
    }
}

void try_compact() {
    if (arena != 0 && freemem != 0) {
       struct myinfo stat = myinfo();
       double fragm = ((double)(stat.freemem - stat.maxfreeblk) * 100) / stat.freemem;
      if (fragm > 70) compact();
    }
}

void* internal_allocate(size_t size) {
    void* ptr = get_block(alloc_align(size));
    if (ptr == NULL) {
        compact();
        ptr = get_block(alloc_align(size));
    }
    try_compact();
    return ptr; 
}

void internal_free(void* ptr) {
    item_p item = (item_p) ptr;
    set_free(item);
    freeblks++;
    usdblks--;
    freemem += block_size(item);
    usdmem -= block_size(item);
    try_compact();
}

