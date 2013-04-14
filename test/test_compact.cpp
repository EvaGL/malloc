#include <compact.h>
#include <stdio.h>
struct s{int a; int b;};
int main() {
    int n ;
    scanf("%d", &n);
    heap_pointer<int> a = allocate<int>(n);
    for (int i = 0; i < n; ++i)
        a[i] = i;
    int sum = 0;
    for (int i = 0; i < n; ++i)
        sum += *(a[i]);
    printf("%d\n", sum);
    free(a);

    heap_pointer<s> ps = allocate<s>(1);
    ps->a = 1;
    ps->b = 2;
    printf("%d\n", ps->a);
    printf("%d\n", ps->b);
}
