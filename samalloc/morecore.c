#include "debug.h"
#include "morecore.h"
#include "heap.h"
#include "config.h"

#ifdef MAX_HEAP_SIZE
    static int can_allow = MAX_HEAP_SIZE;
#endif

header_p morecore(size_t size)
{
    MDEBUG("morecore: try to alloc %d\n", size);
    size_t s = sbrk_align(size);
    #ifdef MAX_HEAP_SIZE
    if (can_allow <= 0)
        return NULL;
    if (s < can_allow)
        s = can_allow;
    can_allow -= s;
    #endif
    header_p heap_br = sbrk(0);
    if (sbrk(s) == (void *)(-1))
        return NULL;
    set_size(heap_br, s - META_SIZE);
    set_free(heap_br);
    MDEBUG("morecore: create block %p with size=%d\n", heap_br, block_size(heap_br));
    return heap_br;
}
