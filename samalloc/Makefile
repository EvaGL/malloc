main: libsamalloc.so malloc.so
	
libsamalloc.so: samalloc.o fit_malloc.o morecore.o
	gcc -shared -g -lpthread -o libsamalloc.so samalloc.o fit_malloc.o morecore.o
malloc.so: malloc.o fit_malloc.o morecore.o
	gcc -shared -g -lpthread -o malloc.so malloc.o fit_malloc.o morecore.o
samalloc.o: samalloc.c heap.h config.h debug.h 
	gcc -c -g -fpic samalloc.c
malloc.o: malloc.c heap.h config.h debug.h 
	gcc -c -g -fpic malloc.c
fit_malloc.o: fit_malloc.c fit_malloc.h heap.h config.h debug.h morecore.h
	gcc -c -g -fpic fit_malloc.c
morecore.o: morecore.c morecore.h heap.h config.h debug.h
	gcc -c -g -fpic morecore.c
clean:
	rm *.o
	rm *.so
