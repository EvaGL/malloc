#include <malloc.h>
#include "config.h"


#ifdef FITTING
    #include "fit_malloc.h"
    #define __malloc(s) fit_malloc(s)
    #define __free(p) fit_free(p)
#endif

void *malloc(size_t size, const void* caller)
{
    void* ans = __malloc(size);
//    print_heap_dump();
    return ans;
}

void free(void *ptr, const void* caller)
{
    __free(ptr);
}
/*
void calloc(size_t nmemb, size_t lsize)
{
    void* result;
    size_t size = nmemb * lsize;
    if (nmemb && (size / nmemb) != lsize)
        return NULL;
    if ((result = __malloc(size)) != NULL) {
        memset(result, 0, size);
    }
    return result;
}

void realloc(void *ptr, size_t size)
{
    if (!size)
    {
        __free(ptr);
        return __malloc(size);
    }
    if (!ptr)
        return __malloc(size);
    __free(ptr);
    return __malloc(size);
}
*/
