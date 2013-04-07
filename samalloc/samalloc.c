#include "config.h"
#include "debug.h"

#ifdef FITTING
    #include "fit_malloc.h"
#endif
              
void *samalloc(size_t size)
{
    MDEBUG("Allocate %d byte\n", size);
    void* ans = __malloc(size);
    return ans;
}

void safree(void *ptr)
{
    if (!ptr)
        return;
    MDEBUG("free %p\n", ptr);
    __free(ptr);
}

void *sacalloc(size_t nmemb, size_t lsize)
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
    return result;
}

void *sarealloc(void *ptr, size_t size)
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

struct mallinfo samallinfo() {
    return __mallinfo();
}

void print_heap_dump(){
  __print_heap_dump();
}

struct myinfo myinfo() {
    return __myinfo();
}
