#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

// ./FACTORIAL -k 12 --pnum=3 --mod=2
long long result = 1;
pthread_mutex_t mut;

typedef struct {
    int from;
    int to;
    int mod;
} Args;

void* calculate_part(void* arg) {
    Args* a = (Args*)arg;
    
    long long local_result = 1;
    
    for (int i = a->from; i <= a->to; i++) {
        if (i % a->mod == 0) {
            local_result *= i;
            printf("умножено на %d\n", i);
        }
    }
    
    pthread_mutex_lock(&mut);
    
    result = result * local_result;
    
    pthread_mutex_unlock(&mut);
    
    free(a);
    
    return NULL;
}

int main(int argc, char* argv[]) {
    int k = 0, pnum = 1, mod = 1;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-k") == 0) {
            k = atoi(argv[i+1]);
            i++;
        }
        if (strstr(argv[i], "--pnum=") != NULL) {
            pnum = atoi(argv[i] + 7);
        }
        if (strstr(argv[i], "--mod=") != NULL) {
            mod = atoi(argv[i] + 6);
        }
    }
    
    
    pthread_t* threads = (pthread_t*)malloc(pnum * sizeof(pthread_t));
    
    int step = k / pnum;
    int ost = k % pnum;
    int start = 1;
    
    for (int i = 0; i < pnum; i++) {
        Args* a = (Args*)malloc(sizeof(Args));
        a->mod = mod;
        a->from = start;
        
        a->to = start + step - 1;
        if (i < ost) {
            a->to++;
        }
        
        if (a->to > k) {
            a->to = k;
        }
        
        pthread_create(&threads[i], NULL, calculate_part, a);
        printf("Создан поток с нач %d, ост %d\n", a->from, a-> to);
    
        start = a->to + 1;
    }
    
    for (int i = 0; i < pnum; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Результат: %lld\n", result);
    

    free(threads);
    return 0;
}