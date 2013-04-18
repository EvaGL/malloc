#include "../include/compact.h"
#include <sys/mman.h>
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
    handler_p page = (handler_p) mmap(0, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    page->next = NULL;
    page->last = ((void*) page) + PAGE_SIZE;
    page->size = 0;

}

void init_heap() {
    heap = new_page();
}

void print_heap_dump() {
    printf("========= HEAP DUMP =========\n");
    handler_p curr_h = heap;
    while (curr_h != NULL) {
        for (int i = 0; i < curr_h->size; ++i) {
            item& curr = curr_h->items[i];
            if (curr.size != 0 && curr.data != NULL) {
                printf("%p %c %7db\n", curr.data, is_free(curr) ? 'f' : 'u', curr.size & (~1));
            }
        }
        curr_h = curr_h->next;
    }
    printf("=============================\n");
}
void compact() {
    void *threshold = NULL;
    void* last = NULL;
    handler_p curr_h = heap;
    while (curr_h != NULL) {
        for (int i = 0; i < curr_h->size; ++i) {
            item& header = curr_h->items[i];
            last = (char*)header.data + header.size; 
            if (!is_free(header)) {
                if (threshold == NULL)
                    continue;
                char* a = (char*) threshold;
                char* b = (char*) header.data;
                for (int i = 0; i < header.size; ++i) {
                    *a = *b;
                    ++a; ++b;
                }
                header.data = threshold;
                threshold = (void*) a;
            } else{
                if (threshold == NULL)
                    threshold = header.data;
                header.data = NULL;
            }
        }
        if (curr_h->next == NULL) {
            void* data = threshold;
            size_t size = (char*)last - (char*)threshold;
            int i = curr_h->size;
            if (curr_h->items + i == curr_h->last) {
                curr_h->next = new_page();
                curr_h = curr_h->next;
                i = 1;
            }
            curr_h->size++;
            item& free_item = curr_h->items[i];
            free_item.size = size;
            set_free(free_item);
            free_item.data = data;
        }
        curr_h = curr_h->next;
    }
}

void* get_block(size_t size) {
    handler_p curr_handler = heap;
    while (curr_handler != NULL) {
        int index = 0;
        while (index < curr_handler->size) {
            item& header = curr_handler->items[index];
            if (block_size(header) >= size && is_free(header)) {  
                set_used(header);
                return &header;
            }
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
    if (ptr == NULL) {
        compact();
        ptr = get_block(alloc_align(size));
    }
    return ptr; 
}

void internal_free(void* ptr) {
    item_p item = (item_p) ptr;
    set_free(*item);
}

