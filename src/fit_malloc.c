#include <assert.h>

#include "config.h"
#include "debug.h"
#include "heap.h"
#include "morecore.h"
#include "fit_malloc.h"

#if USE_MUTEX == 1
    #define __USE_GNU
    #include <pthread.h>
    pthread_mutex_t lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
    #define heap_lock(lock) pthread_mutex_lock(&lock)
    #define heap_unlock(lock) pthread_mutex_unlock(&lock)
#else
    #define heap_lock(lock) (void) 0
    #define heap_unlock(lock) (void) 0
#endif

header_p heap = NULL;
header_p used_heap = NULL;

header_p first_block = NULL;
header_p last_block = NULL;

size_t used_bl;
size_t free_bl;
size_t used_mem;
size_t free_mem;
size_t total_mem;

static struct free_block_header null_block;

struct mallinfo fit_mallinfo() {
    struct mallinfo mi;
    int keepcost = ((void*)footer(last_block)) - ((void*) first_block);
    mi = (struct mallinfo){total_mem, free_bl, 0, 0, 0, 0, 0, used_mem, free_mem, keepcost};
    return mi;
}

struct myinfo fit_myinfo() {
    struct myinfo mi;
    mi = (struct myinfo){total_mem, free_mem, used_mem, free_bl, used_bl, 0};
    header_p curr = heap;
    while (curr != NULL) {
        int s = block_size(curr);
        if (is_free(curr) && s > mi.maxfreeblk)
        {
            mi.maxfreeblk = s;
        }
        if (curr->next == heap)
            break;
        curr = curr->next;
    }
    return mi; 
}


void init_heap()
{
    used_bl = 0;
    free_bl = 0;
    used_mem = 0;
    free_mem = 0;
    total_mem = 0;
    null_block.size = 0;
    #ifndef ROUND_ROBIN
    null_block.prev = NULL;
    null_block.next = NULL;
    #else
    null_block.prev = &null_block;
    null_block.next = &null_block;
    #endif
    heap = &null_block;
}


void fit_print_heap_dump()
{
    MDEBUG("=========== STATS ==============\n");
    MDEBUG("Num of used chunks : %d\n", used_bl);
    MDEBUG("Num of free chunks : %d\n", free_bl);
    MDEBUG("Total memory       : %d\n", total_mem);
    MDEBUG("Free memory        : %d\n", free_mem);
    MDEBUG("Used memory        : %d\n", used_mem);
    MDEBUG("========= HEAP DUMP ============\n");
    header_p curr = first_block;
    header_p last = ((void*)last_block) + block_size(last_block) + META_SIZE;
    for (; curr < last; curr = ((void*)curr) + block_size(curr) + META_SIZE)
    {
        MDEBUG("%p-%p %c %4db at %p-%p\n", curr, (void*)(footer(curr)) + FOOTER_SIZE - 1,
        (is_free(curr)) ? 'f' : 'u', block_size(curr), payload(curr), ((void*)footer(curr)) - 1);       
    }
    MDEBUG("================================\n");
    #ifdef FIT_DELETE_USED
    curr = heap;
    while(1)
    {
        if (block_size(curr) != NULL)
        MDEBUG("%p-%p %c %4db at %p-%p\n", curr, (void*)(footer(curr)) + FOOTER_SIZE - 1,
        (is_free(curr)) ? 'f' : 'u', block_size(curr), payload(curr), ((void*)footer(curr)) - 1);       
        curr = curr->next;
        if (curr == NULL || curr == heap)
            break;
    }
    MDEBUG("================================\n");
    curr = used_heap;
    for (; curr != NULL; curr = curr->next)
    {
        MDEBUG("%p-%p %c %4db at %p-%p\n", curr, (void*)(footer(curr)) + FOOTER_SIZE - 1,
        (is_free(curr)) ? 'f' : 'u', block_size(curr), payload(curr), ((void*)footer(curr)) - 1);       
    }
    MDEBUG("================================\n");
    #endif
}

void split_block(header_p h, size_t size)
{
    header_p new_block = payload(h) + size + FOOTER_SIZE;
    set_size(new_block, block_size(h) - size - META_SIZE);
    set_size(h, size);
    insert_block(&heap, new_block, h, h->next);
    set_free(new_block);
    if (last_block == h)
        last_block = new_block;
    MDEBUG("split block %p to %p(%d) and %p(%d)\n", h, h, size, new_block, block_size(new_block)); 
    // stats
    free_bl++;
    free_mem -= META_SIZE;
}

header_p merge_block(header_p h)
{
#ifdef ALLOW_MERGE
    if (!is_free(h))
        return;
    if (h != last_block)
    {
        header_p next = ((void *) h) + block_size(h) + META_SIZE;
        MDEBUG("merge: next - %p\n", next);
        if (is_free(next)) 
        {
            if (next->next != NULL)
                next->next->prev = next->prev;
            if (next->prev != NULL)
                next->prev->next = next->next;
            set_size(h, block_size(h) + block_size(next) + META_SIZE);
            set_free(h);
            if (next == last_block)
                last_block = h;
            if (heap == next)
                heap = h;
            MDEBUG("merge blocks %p and %p\n", h, next);
            // stats
            free_bl--;
            free_mem += META_SIZE;
        }
    }
    if (h != first_block)
    {
        footer_p prev_foot = ((void *) h) - FOOTER_SIZE;
        header_p prev = ((void*) prev_foot) - prev_foot->size - HEADER_SIZE;
        MDEBUG("merge: prev - %p\n", prev);
        if (is_free(prev))
        {
            if (h->next != NULL)
                h->next->prev = h->prev;
            if (h->prev != NULL)
                h->prev->next = h->next;
            set_size(prev, block_size(prev) + block_size(h) + META_SIZE);
            set_free(prev);
            if (h == last_block)
                last_block = prev;
            if (heap == h)
                heap = prev;
            MDEBUG("merge blocks %p and %p\n", prev, h);
            // stats
            free_bl--;
            free_mem += META_SIZE;
            h = prev;
        }
    }
#endif
    return h;
}

header_p find_fit_block(size_t size)
{
    header_p curr = heap;
    #ifdef ROUND_ROBIN
    header_p last = heap;
    #else
    header_p last = NULL;
    #endif
    int find_one = 0;
    header_p our_choose = NULL;
    int fitting_param;
    while (curr != NULL)
    {
        if (is_free(curr) && block_size(curr) >= size)
        {
            #ifdef BEST_FIT
            if (!find_one || fitting_param > block_size(curr))
            #endif
            #ifdef WORST_FIT
            if (!find_one || fitting_param < block_size(curr))
            #endif
            {
                our_choose = curr;
                fitting_param = block_size(curr);
            }
            find_one = 1; 
            #ifdef FIRST_FIT
            break;
            #endif
        }
        if (curr->next == last)
            break;
        curr = curr->next;
    }

    if (!find_one)
    {
        size_t p_size = page_align(size + META_SIZE);
        our_choose = morecore(p_size);
        if (our_choose == NULL)
            return NULL;
        total_mem += block_size(our_choose) + META_SIZE;
        free_mem += block_size(our_choose);
        free_bl++;
        if (first_block == NULL)
            first_block = our_choose;
        last_block = our_choose;
        insert_block(&heap, our_choose, curr, last);
        our_choose = merge_block(our_choose);
    }
    #ifdef ROUND_ROBIN
    heap = our_choose->next;
    #endif
    return our_choose;
}

void* fit_malloc(size_t size)
{
    MDEBUG("fit_malloc: size= %d\n", size); 
    size_t s = alloc_align(size + META_SIZE);   
    size = alloc_align(size);
    header_p block;
    heap_lock(lock);
    if (heap == NULL)
    {
        init_heap();
    }
    block = find_fit_block(s);
    if (block == NULL) 
    {
        heap_unlock(lock);
        return NULL;
    }

    if (block_size(block) > size + META_SIZE)
        split_block(block, size);
    set_used(block);
    
    #ifdef FIT_DELETE_USED
    delete_block(&heap, block, block->prev, block->next);
    insert_block(&used_heap, block, NULL, used_heap);
    #endif
    // stats
    used_bl++;
    free_bl--;
    used_mem += block_size(block);
    free_mem -= block_size(block);
    // stats
    MDEBUG("fit_malloc: returning %p\n", payload(block));
   
    if (!(block_size(block) >= size &&
        payload(block) >= (void*) first_block &&
        (void*) footer(block) <= sbrk(0)))
    {
        MDEBUG("Malloc error!!! %d\n", size);
        fit_print_heap_dump();
        exit(1);
    }
    heap_unlock(lock);

    return payload(block);
}

void fit_free(void* ptr)
{
    heap_lock(lock);
    MDEBUG("fit_free: ptr= %p\n", ptr);
    header_p last = NULL;
    #ifndef FIT_DELETE_USED
    header_p curr = heap;
    #else
    header_p curr = used_heap;
    #endif
    for(; curr != last; curr = curr->next)
        if (payload(curr) == ptr)
        {
            MDEBUG("fit_free: set block %p with size=%d as free\n", curr, block_size(curr));
            set_free(curr);
            // stats
            free_bl++;
            used_bl--;
            used_mem -= block_size(curr);
            free_mem += block_size(curr);
            // -----
            #ifdef FIT_DELETE_USED
            delete_block(&used_heap, curr, curr->prev, curr->next); 
            insert_block(&heap, curr, heap, heap->next);
            #endif
            merge_block(curr);

            heap_unlock(lock);
            return;
        }
    MDEBUG("Invalid pointer!!! %p\n", ptr);
    fit_print_heap_dump();
    exit(1);
}

