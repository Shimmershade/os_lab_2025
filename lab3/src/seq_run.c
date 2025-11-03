#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <seed> <array_size>\n", argv[0]);
        return 1;
    }

    int seed = atoi(argv[1]);
    int array_size = atoi(argv[2]);

    if (seed <= 0 || array_size <= 0) {
        printf("Seed and array size must be positive numbers\n");
        return 1;
    }

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    pid_t pid = fork();
    
    if (pid == -1) {
        printf("Fork failed!\n");
        return 1;
    } else if (pid == 0) {

        char seed_str[20];
        char array_size_str[20];
        
        sprintf(seed_str, "%d", seed);
        sprintf(array_size_str, "%d", array_size);
        
        // ВОТ ТУТ ЭКЗЕК
        char *args[] = {"./sequential_min_max", seed_str, array_size_str, NULL};
        execv(args[0], args);
        
        printf("Failed to execute sequential_min_max\n");
        exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);
        
        gettimeofday(&end_time, NULL);
        
        double elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0;
        elapsed_time += (end_time.tv_usec - start_time.tv_usec) / 1000.0;
        
        if (WIFEXITED(status)) {
            printf("sequential_min_max completed successfully\n");
            printf("Time: %.2f ms\n", elapsed_time);
        } else {
            printf("sequential_min_max failed\n");
        }
    }

    return 0;
}