#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <pthread.h>

#include "sum_lib.h"

void print_usage() {
    printf("Usage: ./psum --threads_num <num> --seed <num> --array_size <num>\n");
}

void GenerateArray(int *array, unsigned int array_size, unsigned int seed) {
  srand(seed);
  for (unsigned int i = 0; i < array_size; i++) {
    array[i] = rand();
  }
}

int parse_args(int argc, char **argv, uint32_t *threads_num, uint32_t *seed, uint32_t *array_size) {
    if (argc != 7) {
        print_usage();
        return -1;
    }

    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "--threads_num") == 0) {
            *threads_num = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "--seed") == 0) {
            *seed = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "--array_size") == 0) {
            *array_size = atoi(argv[i + 1]);
        } else {
            print_usage();
            return -1;
        }
    }

    if (*threads_num <= 0 || *array_size <= 0) {
        printf("Error: threads_num and array_size must be positive numbers\n");
        return -1;
    }

    return 0;
}

int main(int argc, char **argv) {
    uint32_t threads_num = 0;
    uint32_t array_size = 0;
    uint32_t seed = 0;

    if (parse_args(argc, argv, &threads_num, &seed, &array_size) != 0) {
        return 1;
    }

    int *array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    pthread_t threads[threads_num];
    struct SumArgs args[threads_num];

    int chunk_size = array_size / threads_num;
    int remainder = array_size % threads_num;
    int current_start = 0;

    for (uint32_t i = 0; i < threads_num; i++) {
        args[i].array = array;
        args[i].begin = current_start;
        args[i].end = current_start + chunk_size + (i < remainder ? 1 : 0);
        current_start = args[i].end;

        if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i])) {
            printf("Error: pthread_create failed!\n");
            free(array);
            return 1;
        }
    }

    int total_sum = 0;
    for (uint32_t i = 0; i < threads_num; i++) {
        int sum = 0;
        pthread_join(threads[i], (void **)&sum);
        total_sum += sum;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = (end.tv_sec - start.tv_sec) + 
                         (end.tv_nsec - start.tv_nsec) / 1e9;

    free(array);
    printf("Total: %d\n", total_sum);
    printf("Elapsed time: %.6f seconds\n", elapsed_time);
    
    return 0;
}