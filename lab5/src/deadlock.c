#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int A = 0;
int B = 0;
pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;

void* function1(void *arg);
void* function2(void *arg);
  
int main() {
    pthread_t thread1, thread2;
    
    pthread_create(&thread1, NULL, function1, NULL);
    pthread_create(&thread2, NULL, function2, NULL);
    
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    printf("Эта строка никогда не будет выведена из-за deadlock\n");
    return 0;
}

void* function1(void *arg) {
    printf("Поток 1 пытается захватить lock1\n");
    pthread_mutex_lock(&lock1);
    printf("Поток 1 захватил lock1\n");
    
    sleep(1);
    
    printf("Поток 1 пытается захватить lock2\n");
    pthread_mutex_lock(&lock2);
    printf("Поток 1 захватил 2 мьютекса\n");
    
    for (int i = 0; i < 5; i++) {
        A += B;
        printf("Поток 1: A = %d, B = %d\n", A, B);
        sleep(1);
    }
    B += A;
    
    printf("Поток 1 завершен\n");
    pthread_mutex_unlock(&lock2);
    pthread_mutex_unlock(&lock1);
    
    return NULL;
} 

void* function2(void *arg) {
    printf("Поток 2 пытается захватить мьютекс 2\n");
    pthread_mutex_lock(&lock2);
    printf("Поток 2 lock2 захвачен\n");
    
    sleep(1);
    
    printf("Поток 2 пытается захватить мьютекс 1\n");
    pthread_mutex_lock(&lock1);
    printf("Поток 2 захватил 2 мьютекса\n");
    
    for (int i = 0; i < 5; i++) {
        B += A;
        printf("Поток 2: A = %d, B = %d\n", A, B);
        sleep(1);
    }
    A += B;
    
    printf("Поток 2 завершен\n");
    pthread_mutex_unlock(&lock1);
    pthread_mutex_unlock(&lock2);
    
    return NULL;
}