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
header_p first_block = NULL;
header_p last_block = NULL;

size_t used_bl;
size_t free_bl;
size_t used_mem;
size_t free_mem;
size_t total_mem;

struct mallinfo
{
  int arena;    /* non-mmapped space allocated from system */
  int ordblks;  /* number of free chunks */
  int smblks;   /* number of fastbin blocks */
  int hblks;    /* number of mmapped regions */
  int hblkhd;   /* space in mmapped regions */
  int usmblks;  /* maximum total allocated space */
  int fsmblks;  /* space available in freed fastbin blocks */
  int uordblks; /* total allocated space */
  int fordblks; /* total free space */
  int keepcost; /* top-most, releasable (via malloc_trim) space */
};

struct mallinfo mallinfo() {
    struct mallinfo mi;
    int keepcost = ((void*)footer(last_block)) - ((void*) first_block);
    mi = (struct mallinfo){total_mem, free_bl, 0, 0, 0, 0, 0, used_mem, free_mem, keepcost};
    return mi;
}
void init_heap()
{
    used_bl = 0;
    free_bl = 0;
    used_mem = 0;
    free_mem = 0;
    total_mem = 0;
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
    header_p curr = heap;
    header_p last = NULL;
    for (; curr != last; curr = curr->next)
    {
        MDEBUG("%p-%p %c %4db at %p-%p\n", curr, (void*)(footer(curr)) + FOOTER_SIZE - 1,
        (is_free(curr)) ? 'f' : 'u', block_size(curr), payload(curr), ((void*)footer(curr)) - 1);       
    }
    MDEBUG("================================\n");
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

void merge_block(header_p h)
{
    if (!is_free(h))
        return;
    if (h != last_block)
    {
        header_p next = ((void *) h) + block_size(h) + META_SIZE;
        MDEBUG("merge: next - %p\n", next);
        if (is_free(next)) 
        {
            h->next = next->next;
            set_size(h, block_size(h) + block_size(next) + META_SIZE);
            set_free(h);
            if (next == last_block)
                last_block = h;
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
            prev->next = h->next;
            set_size(prev, block_size(prev) + block_size(h) + META_SIZE);
            set_free(prev);
            if (h == last_block)
                last_block = prev;
            MDEBUG("merge blocks %p and %p\n", prev, h);
            // stats
            free_bl--;
            free_mem += META_SIZE;
        }
    }
}

header_p find_fit_block(size_t size)
{
    header_p curr = heap;
    header_p last = NULL;
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
            #ifdef FIRST_FIT
            break;
            #endif
            find_one = 1; 
        }
        if (curr->next == last && !find_one)
        {
            header_p new_block = morecore(size + META_SIZE);
            if (new_block == NULL)
                return NULL;
            // stats
            total_mem += block_size(new_block) + META_SIZE;
            free_mem += block_size(new_block);
            free_bl++;
            //-----
            last_block = new_block;
            insert_block(&heap, new_block, curr, last);
            if (is_free(curr))
            {
                merge_block(curr);
                continue;
            }
        }
        curr = curr->next;
    }
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
        block = morecore(s);
        if (block == NULL)
        {
            heap_unlock(lock);
            return NULL;
        }
        // stats
        total_mem += block_size(block) + META_SIZE;
        free_mem += block_size(block);
        free_bl++;
        // stats
        insert_block(&heap, block, NULL, NULL);
        first_block = block;
        last_block = block;
    } else {
        block = find_fit_block(s);
        if (block == NULL) 
        {
            heap_unlock(lock);
            return NULL;
        }
    }

    if (block_size(block) > size + META_SIZE)
        split_block(block, size);
    set_used(block);
    // stats
    used_bl++;
    free_bl--;
    used_mem += block_size(block);
    free_mem -= block_size(block);
    // stats
    heap_unlock(lock);
    MDEBUG("fit_malloc: returning %p\n", payload(block));
    
    assert(block_size(block) >= size);
    assert(payload(block) >= (void*)first_block);
    assert((void*) footer(block) <= sbrk(0));

    return payload(block);
}

void fit_free(void* ptr)
{
    heap_lock(lock);
    MDEBUG("fit_free: ptr= %p\n", ptr);
    header_p last = NULL;
    header_p curr = heap;
    for(; curr != last; curr = curr->next)
        if (payload(curr) == ptr)
        {
            MDEBUG("fit_free: set block %p with size=%d as free\n", curr, block_size(curr));
            set_free(curr);
            // stats
            free_bl++;
            used_bl++;
            used_mem -= block_size(curr);
            free_mem += block_size(curr);
            // -----
            merge_block(curr);
            heap_unlock(lock);
            return;
        }
    MDEBUG("Invalid pointer!!! %p\n", ptr);
    fit_print_heap_dump();
    exit(1);
}

