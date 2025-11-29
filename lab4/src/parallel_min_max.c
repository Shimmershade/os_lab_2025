#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"


// ./parallel_min_max --seed 4 --array_size 8 --pnum 2 --timeout=20 --by_files 
volatile sig_atomic_t print_flag = false;

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  int timeout = -1;
  bool with_files = false;

  static struct option options[] = {
      {"seed", required_argument, 0, 0},
      {"array_size", required_argument, 0, 0},
      {"pnum", required_argument, 0, 0},
      {"timeout", optional_argument, 0, 0},
      {"by_files", no_argument, 0, 'f'},
      {0, 0, 0, 0}
  };

  while (true) {
    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) {
                printf("seed must be a positive number\n");
                return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
                printf("Array_size must be a positive number\n");
                return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0) {
                printf("Pnum must be a positive number\n");
                return 1;
            }
            break;
          case 3:
            if (optarg != NULL) {
                timeout = atoi(optarg);
                if (timeout <= 0) {
                    printf("Timeout must be a positive number\n");
                    return 1;
                }
            } else {
                timeout = 10;
            }
            break;
          case 4:
            with_files = true;
            break;
          default:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;
      case '?':
        printf("Unknown option\n");
        return 1;
      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument: %s\n", argv[optind]);
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--timeout [\"num\"]] [--by_files]\n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  int part_size = array_size / pnum;

  // СОЗДАНИЕ ПАЙПОВ
  int pipe_fd[2];
  if (!with_files) {
    if (pipe(pipe_fd) == -1) {
        printf("Pipe creation failed\n");
        free(array);
        return 1;
    }
  }

  // Массив для хранения PID дочерних процессов
  pid_t *child_pids = malloc(sizeof(pid_t) * pnum);
  
  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      active_child_processes += 1;
      child_pids[i] = child_pid;
      
      if (child_pid == 0) {
        unsigned int begin = i * part_size;
        unsigned int end = (i == pnum - 1) ? array_size : (i + 1) * part_size;
        struct MinMax local_min_max = GetMinMax(array, begin, end);

        if (with_files) {
            char filename[256];
            snprintf(filename, sizeof(filename), "minmax_%d_%d.txt", getpid(), i);
            FILE *file = fopen(filename, "w");
            if (file == NULL) {
                perror("Failed to open file");
                exit(1);
            }
            fprintf(file, "%d %d", local_min_max.min, local_min_max.max);
            fclose(file);
        } else {
            close(pipe_fd[0]);
            if (write(pipe_fd[1], &local_min_max, sizeof(struct MinMax)) != sizeof(struct MinMax)) {
                perror("Write to pipe failed");
            }
            close(pipe_fd[1]);
        }
        exit(0);
      }
    } else {
      printf("Fork failed!\n");
      free(array);
      free(child_pids);
      return 1;
    }
  }

  if (!with_files) {
    close(pipe_fd[1]);
  }

  if (timeout != -1) {
    printf("Using timeout: %d seconds\n", timeout);
    
    alarm(timeout);
    signal(SIGALRM, );
    
    if (active_child_processes > 0) { 
      for (int i = 0; i < pnum; i++) {
        if (kill(child_pids[i], 0) == 0) {
          if (kill(child_pids[i], SIGKILL) == 0) {
            printf("Sent SIGKILL to process %d\n", child_pids[i]);
          }
        }
      }
      
      while (active_child_processes > 0) {
        wait(NULL);
        active_child_processes -= 1;
      }
    }
  } else {
    while (active_child_processes > 0) {
      wait(NULL);
      active_child_processes -= 1;
    }
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      char filename[256];
      snprintf(filename, sizeof(filename), "minmax_%d_%d.txt", child_pids[i], i);
      FILE *file = fopen(filename, "r");
      if (file == NULL) {
          printf("Failed to open file %s: ", filename);
          perror("");
          continue;
      }
      if (fscanf(file, "%d %d", &min, &max) != 2) {
          printf("Failed to read from file %s\n", filename);
          fclose(file);
          continue;
      }
      fclose(file);
      remove(filename);
    } else {
      struct MinMax local_min_max;
      if (read(pipe_fd[0], &local_min_max, sizeof(struct MinMax)) == sizeof(struct MinMax)) {
        min = local_min_max.min;
        max = local_min_max.max;
      }
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  if (!with_files) {
    close(pipe_fd[0]);
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  free(child_pids);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  return 0;
}