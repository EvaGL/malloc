#ifndef _SAMALLOC_H_
#define _SAMALLOC_H_
struct myinfo
{
    int arena;
    int freemem;
    int usdmem;
    int freeblks;
    int usdblks;
    int maxfreeblk;
};

void print_heap_dump();
struct myinfo myinfo();
void *samalloc(size_t size);
void safree(void *ptr);
void *sacalloc(size_t nmemb, size_t lsize);
void *sarealloc(void* ptr, size_t size);
#endif
