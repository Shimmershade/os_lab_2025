CC=gcc
CFLAGS=-pthread
TARGET=psum
LIB=libsum.a

all: $(TARGET)

$(TARGET): main.o libsum.a
	$(CC) $(CFLAGS) -o $(TARGET) main.o -L. -lsum

main.o: parallel_sum.c sum_lib.h
	$(CC) $(CFLAGS) -c parallel_sum.c -o main.o

libsum.a: sum_lib.o
	ar rcs $(LIB) sum_lib.o

sum_lib.o: sum_lib.c sum_lib.h
	$(CC) $(CFLAGS) -c sum_lib.c -o sum_lib.o

clean:
	rm -f *.o $(TARGET) $(LIB)

.PHONY: all clean