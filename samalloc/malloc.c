#include "config.h"
#include "debug.h"
//#include <malloc.h>

#ifdef FITTING
    #include "fit_malloc.h"
#endif
              
void *malloc(size_t size)
{
    MDEBUG("Allocate %d byte\n", size);
    void* ans = __malloc(size);
    __print_heap_dump();
    return ans;
}

void free(void *ptr)
{
    if (!ptr)
        return;
    MDEBUG("free %p\n", ptr);
    __free(ptr);
    __print_heap_dump();
}

void *calloc(size_t nmemb, size_t lsize)
{
    void* result;
    size_t size = nmemb * lsize;
    if (nmemb && (size / nmemb) != lsize)
    {
        return NULL;
    }
    if ((result = __malloc(size)) != NULL) {
        memset(result, 0, size);
    }
    __print_heap_dump();
    return result;
}

void *realloc(void *ptr, size_t size)
{
    if (!size)
    {
        __free(ptr);
        return  __malloc(size);
    }
    if (!ptr)
    {
        return  __malloc(size);
    }
    __free(ptr);
    return  __malloc(size);
}
/*
static void my_init_hook (void)
{
    __malloc_hook = samalloc;
    __realloc_hook = sarealloc;
    __free_hook = safree;
}
void (*__MALLOC_HOOK_VOLATILE __malloc_initialize_hook) (void) = my_init_hook;
*/
