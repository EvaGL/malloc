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
    void *threshold = heap;
    void *curr = threshold;
    void* last = NULL;
    while (curr != NULL) {
        if (threshold < curr_handler) {
            int* a = (int*)threshold;
            int* b = (int*)curr;
            for (int i = 0; i < PAGE_SIZE / sizeof(int); ++i) {
                *a = *b;
                ++a; ++b;
            }
            curr = threshold;
            threshold = (void*) a;
            ((handler_p) curr)->last = b;
            last = b;
        }
        handler_p curr_h = (handler_p) curr;
        for (int i = 0; i < curr_h->size; ++i) {
            item header = curr_h->items[i];
            last = header.data + header.size;
            if (is_free(header)) {
                int* a = (int*) threshold;
                int* b = (int*) header.data;
                for (int i = 0; i < header.size/ sizeof(int); ++i) {
                    *a = *b;
                    ++a; ++b;
                }
                header.data = threshold;
                threshold = (void*) a;
            }
        }
        if (curr_h->next == NULL) {
            void* data = threshold;
            size_t size = last - threshold;
            if (curr_h->items + curr_h->size != curr_h->last) {
                item& header = curr_h->items[++curr_h->size];
                header.data = data;
                header.size = size;
                set_free(header);
            } else {
                handler_p new_page = new_page();
                if (new_page != NULL) {
                    new_page->size = 1;
                    
                }
            }
            return;
        }
        curr = (void*) curr_h->next;
    }
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
    item_p item = (item_p) ptr;
    set_free(item);
}

