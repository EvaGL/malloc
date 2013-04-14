#ifndef _SELFCOMPACT_H_
#define _SELFCOMPACT_H_
#include <stddef.h>
void *compact_malloc(size_t size);
void compact_free(void *ptr);

#define __malloc(s) compact_malloc(s)
#define __free(p) compact_free(s)
#define __print_heap_dump() (void) 0
//#define __mallinfo() (void) 0
//#define __myinfo() (void) 0
#endif
