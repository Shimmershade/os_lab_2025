CC=gcc
CFLAGS=-I.

all : parallel_min_max zombie

parallel_min_max : utils.o find_min_max.o utils.h find_min_max.h
	$(CC) -o parallel_min_max utils.o find_min_max.o parallel_min_max.c $(CFLAGS)

utils.o : utils.h
	$(CC) -o utils.o -c utils.c $(CFLAGS)

find_min_max.o : utils.h find_min_max.h
	$(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)

zombie:
	$(CC) zombie_test.c -o zombie.o

process_memory:
	$(CC) -o process_memory process_memory.c $(CFLAGS)

clean :
	rm *.o parallel_min_max process_memory
