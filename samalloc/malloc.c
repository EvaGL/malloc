#include "config.h"
#include "debug.h"
#include <malloc.h>

#ifdef FITTING
    #include "fit_malloc.h"
    #define __malloc(s) fit_malloc(s)
    #define __free(p) fit_free(p)
#endif
              
void *samalloc(size_t size, const void* caller)
{
    MDEBUG("Allocate %d byte\n", size);
    void* ans = __malloc(size);
    print_heap_dump();
    return ans;
}

void safree(void *ptr, const void* caller)
{
    if (!ptr)
        return;
    MDEBUG("free %p\n", ptr);
    __free(ptr);
    print_heap_dump();
}

void sacalloc(size_t nmemb, size_t lsize, const void* caller)
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
    print_heap_dump();
    return result;
}

void sarealloc(void *ptr, size_t size, const void* caller)
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

static void my_init_hook (void)
{
    __malloc_hook = samalloc;
    __realloc_hook = sarealloc;
    __free_hook = safree;
}
