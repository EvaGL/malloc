#ifndef _FIT_MALLOC_H_
#define _FIT_MALLOC_H_

#include "mallinfo.h"
void *fit_malloc(size_t size);
void fit_free(void* ptr);
void fit_print_heap_dump();
struct mallinfo fit_mallinfo();
#define __malloc(s) fit_malloc(s)
#define __free(p) fit_free(p)
#define __print_heap_dump() fit_print_heap_dump()
#define __mallinfo() fit_mallinfo()
#endif
