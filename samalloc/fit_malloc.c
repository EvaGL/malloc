#include "config.h"
#include "debug.h"
#include "heap.h"
#include "morecore.h"
#include "fit_malloc.h"

#if USE_MUTEX == 1
    #include <pthread.h>
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    #define heap_lock(lock) pthread_mutex_lock(&lock)
    #define heap_unlock(lock) pthread_mutex_unlock(&lock)
#else
    #define heap_lock(lock) (void) 0
    #define heap_unlock(lock) (void) 0
#endif

header_p heap = NULL;

void init_heap()
{
#if USE_MUTEX == 1
    pthread_mutexattr_t mta;
    pthread_mutexattr_init(&mta);
    pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&lock, NULL);//&mta);
    pthread_mutexattr_destroy(&mta);
#endif
}

void print_heap_dump()
{
    MDEBUG("========= HEAP DUMP ============\n");
    header_p curr = heap;
    header_p last = NULL;
    for (; curr != last; curr = curr->next)
    {
        MDEBUG("%p %c size = %d\n", payload(curr), (is_free(curr)) ? 'f' : 'u', block_size(curr));       
    }
    MDEBUG("================================\n");
}

void split_block(header_p h, size_t size)
{
    header_p new_block = payload(h) + size;
    new_block->size = block_size(h) - size - HEADER_SIZE;
    h->size = size;
    insert_block(&heap, new_block, h, h->next);
    set_free(new_block);
    MDEBUG("split block %p to %p(%d) and %p(%d)\n", h, h, size, new_block, block_size(new_block)); 
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
            header_p new_block = morecore(size);
            new_block->size = size;
            set_free(new_block);
            insert_block(&heap, new_block, curr, last);
            if (is_free(curr)) 
            {
                merge_blocks(curr, new_block);
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
    size_t s = alloc_align(size + HEADER_SIZE);   
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
        insert_block(&heap, block, NULL, NULL);
    } else {
        block = find_fit_block(s);
        if (block == NULL) 
        {
            heap_unlock(lock);
            return NULL;
        }
    }

    set_used(block);
    if (block_size(block) > size + HEADER_SIZE)
        split_block(block, size);
    heap_unlock(lock);
    MDEBUG("fit_malloc: returning %p\n", payload(block));
    return payload(block);
}

void fit_free(void* ptr)
{
    heap_lock(lock);
   // MDEBUG("fit_free: ptr= %p\n", ptr);
    header_p last = NULL;
    header_p curr = heap;
    for(; curr != last; curr = curr->next)
        if (payload(curr) == ptr)
        {
    //        MDEBUG("fit_free: set block %p with size=%d as free\n", curr, block_szie(curr));
            set_free(curr);
            break;
        }
    heap_unlock(lock);
}

