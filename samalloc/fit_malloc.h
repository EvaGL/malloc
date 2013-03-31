#ifndef _FIT_MALLOC_H_
#define _FIT_MALLOC_H_

void *fit_malloc(size_t size);
void fit_free(void* ptr);
void print_heap_dump();

#define __malloc(s) fit_malloc(s)
#define __free(p) fit_free(p)
#define __print_heap_dump() fit_print_heap_dump()

#endif
