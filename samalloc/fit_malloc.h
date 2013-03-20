#ifndef _FIT_MALLOC_H_
#define _FIT_MALLOC_H_

void *fit_malloc(size_t size);
void fit_free(void* ptr);
void print_heap_dump();
#endif
