#ifndef _COMPACT_H_
#define _COMPACT_H_

#include <stddef.h>
#include <stdio.h>

void internal_free(void*);

template<typename T>
class heap_pointer{
private:
    void** pointer;
    size_t index;
public:
    heap_pointer() : pointer(NULL) , index(0) {} ;
    
    heap_pointer(void* ptr, size_t i = 0) : pointer((void**)ptr), index(i) {
    }

    T operator*() {
//        printf("get value of %p[%d]\n", *pointer, index);
        return *(T*)(*pointer + index * sizeof(T));
    }
    
    T* operator->() {
        return (T*)(*pointer + index * sizeof(T));
    }

    heap_pointer<T> operator[](size_t index) {
        return heap_pointer((void*)pointer, index);
    }

    void operator=(T elem) {
 //       printf("assign to %p[%d]\n", *pointer, index);
        *(T*)(*pointer + index * sizeof(T)) = elem;
    }
   
    void free() {
        internal_free((void*) pointer);
    }
    
    bool isNull() {
        return pointer == NULL;
    }
};

void* internal_allocate(size_t);
template<typename T>
heap_pointer<T> allocate(size_t num) {
    return heap_pointer<T>(internal_allocate(num * sizeof(T)));
}

struct myinfo
{
    size_t arena;
    size_t freemem;
    size_t usdmem;
    size_t freeblks;
    size_t usdblks;
    size_t maxfreeblk;
};

void print_heap_dump();
struct myinfo myinfo();
#endif
