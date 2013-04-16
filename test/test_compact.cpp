#include <compact.h>
#include <stdio.h>
int main() {
    heap_pointer<int> pointers[10]; 
    int size;
    for (int i = 0; i < 10; ++i) {
        pointers[i] = allocate<int>(100);
    }
    for (int i = 0; i < 10; i += 2) {
        pointers[i].free();
    }
    print_heap_dump();
    allocate<int>(200);
    print_heap_dump();
}
