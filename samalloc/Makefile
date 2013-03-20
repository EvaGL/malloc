malloc.so: malloc.o fit_malloc.o morecore.o
	gcc -shared -g -o malloc.so malloc.o fit_malloc.o morecore.o
malloc.o: malloc.c heap.h config.h debug.h 
	gcc -c -g -fpic malloc.c
fit_malloc.o: fit_malloc.c fit_malloc.h heap.h config.h debug.h morecore.h
	gcc -c -g -fpic fit_malloc.c
morecore.o: morecore.c morecore.h heap.h config.h debug.h
	gcc -c -g -fpic morecore.c
clean:
	rm *.o
	rm *.so
