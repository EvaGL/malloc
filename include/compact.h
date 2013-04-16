#ifndef _COMPACT_H_
#define _COMPACT_H_

#include <stddef.h>
#include <stdio.h>

template<typename T>
class heap_pointer{
private:
    void** pointer;
    size_t index;
public:
    heap_pointer(void* ptr, size_t i = 0) : pointer((void**)ptr), index(i) {
//        printf("new pointer %p[%d]\n", *pointer, index);
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
    
};

void* internal_allocate(size_t);
template<typename T>
heap_pointer<T> allocate(size_t num) {
    return heap_pointer<T>(internal_allocate(num * sizeof(T)));
}

void internal_free(void*);
template<typename T>
void free(heap_pointer<T> ptr) {
   internal_free((void*)*ptr); 
}

void print_heap_dump();
#endif
