#include "../include/compact.h"

extern "C" {
#include "morecore.h"
}

typedef struct item *item_p;
struct item{
    void* data;
    size_t size;
};

static __inline__ size_t block_size(item &h)
{
    return h.size & (~1);
}

static __inline__ int is_free(item& h)
{
    return h.size & 1;
}

static __inline__ void set_free(item& h)
{
    h.size |= 1;
}

static __inline__ void set_used(item& h)
{
    h.size &= (~1);
}


typedef struct handler *handler_p; 
struct handler{
    handler_p next;
    size_t size;
    void* last;
    item items[1];
};

handler_p heap = NULL;

handler_p new_page() {
    handler_p page = (handler_p)morecore(PAGE_SIZE);
    page->next = NULL;
    page->last = ((void*) page) + PAGE_SIZE;
    page->size = 0;

}

void init_heap() {
    heap = new_page();
}

void compact() {

}

void* get_block(size_t size) {
    handler_p curr_handler = heap;
    while (curr_handler != NULL) {
        int index = 0;
        while (index < curr_handler->size) {
            item header = curr_handler->items[index];
            if (block_size(header) <= size && is_free(header))  
                return &header;
            index++;
        }
        if (curr_handler->items + index == curr_handler->last) {
            if (curr_handler->next == NULL) {
                curr_handler->next = new_page();
            }
            curr_handler = curr_handler->next;
        }
        else {
            void* data = morecore(size);
            if (data == NULL)
                return NULL;
            item& new_block = curr_handler->items[index];
            new_block.size = size;
            set_used(new_block);
            new_block.data = data;
            curr_handler->size++; 
            return (void*)&new_block;
        }
    }
    return NULL;
}

void* internal_allocate(size_t size) {
    if (heap == NULL) init_heap();
    void* ptr = get_block(alloc_align(size));
    if (ptr == NULL)
        return NULL;
    return ptr; 
}

void internal_free(void* ptr) {

}

void* malloc(size_t size) {
    return get_block(alloc_align(size));
}

void free(void* ptr) {
}
