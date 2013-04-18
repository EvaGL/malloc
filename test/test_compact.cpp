#include <compact.h>
#include <stdio.h>
int main() {
    heap_pointer<int> pointers[10]; 
    int size;
    for (int i = 0; i < 10; ++i) {
        pointers[i] = allocate<int>(100);
        pointers[i] = i;
    }
    for (int i = 0; i < 10; i += 2) {
        pointers[i].free();
    }
    print_heap_dump();
    for (int i = 1; i < 10; i+= 2) {
        printf("%d ", *(pointers[i]));
    }
    printf("\n");
    allocate<int>(200);
    print_heap_dump();
    for (int i = 1; i < 10; i+= 2) {
        printf("%d ", *(pointers[i]));
    }
    printf("\n");
}
