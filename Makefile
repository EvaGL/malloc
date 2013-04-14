main: lib lib/libsamalloc.so lib/malloc.so lib/libcompmalloc.so
	rm *.o
lib/libsamalloc.so: samalloc.o fit_malloc.o morecore.o
	gcc -shared -g -lpthread -o lib/libsamalloc.so samalloc.o fit_malloc.o morecore.o
lib/malloc.so: malloc.o fit_malloc.o morecore.o selfcompact.o
	gcc -shared -g -lpthread -o lib/malloc.so malloc.o fit_malloc.o morecore.o
lib/libcompmalloc.so: selfcompact.o morecore.o
	g++ -shared -g -lpthread -o lib/libcompmalloc.so selfcompact.o morecore.o
samalloc.o: src/samalloc.c src/heap.h src/config.h src/debug.h 
	gcc -c -g -fpic src/samalloc.c -o samalloc.o
malloc.o: src/malloc.c src/heap.h src/config.h src/debug.h 
	gcc -c -g -fpic src/malloc.c -o malloc.o
fit_malloc.o: src/fit_malloc.c src/fit_malloc.h src/heap.h src/config.h src/debug.h src/morecore.h
	gcc -c -g -fpic src/fit_malloc.c -o fit_malloc.o
selfcompact.o: src/selfcompact.cpp src/selfcompact.h src/heap.h src/debug.h src/morecore.h
	g++ -c -g -fpic src/selfcompact.cpp -o selfcompact.o
morecore.o: src/morecore.c src/morecore.h src/heap.h src/config.h src/debug.h
	gcc -c -g -fpic src/morecore.c -o morecore.o
lib:
	mkdir lib
clean:
	rm lib/*
	rmdir lib
