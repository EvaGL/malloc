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
#endif
