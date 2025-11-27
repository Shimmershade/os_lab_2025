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

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  int timeout = -1;  // Таймаут в секундах
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"timeout", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

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
            timeout = atoi(optarg);
            if (timeout <= 0) {
                printf("Timeout must be a positive number\n");
                return 1;
            }
            break;
          case 4:
            with_files = true;
            break;

          defalut:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--timeout \"num\"] \n",
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
  int pipe_fd[2]; // массив для файловых дескрипторов или как-то так
  if (!with_files) {
    if (pipe(pipe_fd) == -1) {
        printf("Pipe creation failed\n");
        return 1;
    }
  }

  // Массив для хранения PID дочерних процессов
  pid_t *child_pids = malloc(sizeof(pid_t) * pnum);
  
  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // ура вилка работать класс
      active_child_processes += 1;
      child_pids[i] = child_pid;  // Сохраняем PID дочернего процесса
      
      if (child_pid == 0) {
        // мы в ребенке

        unsigned int begin = i * part_size;
        unsigned int end = (i == pnum - 1) ? array_size : (i + 1) * part_size;
        struct MinMax local_min_max = GetMinMax(array, begin, end);

          // use files here
          // господи

          if (with_files) {
            char filename[20];
            printf(filename, "minmax_%d.txt", i);
            FILE *file = fopen(filename, "w");
            if (file == NULL) {
                printf("Failed to open file\n");
                exit(1);
            }
            fprintf(file, "%d %d", local_min_max.min, local_min_max.max);
            fclose(file);
        } else {

            // use pipe here

            close(pipe_fd[0]); // закрыть чтение в ребенке
            write(pipe_fd[1], &local_min_max, sizeof(struct MinMax));
            close(pipe_fd[1]);
        }

        return 0;
      }

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  // Если задан таймаут, устанавливаем обработку
  if (timeout > 0) {
    // Ждем завершения процессов с таймаутом
    int time_elapsed = 0;
    while (active_child_processes > 0 && time_elapsed < timeout) {
      sleep(1);
      time_elapsed++;
      
      // Проверяем, завершились ли какие-то процессы
      pid_t finished_pid;
      int status;
      while ((finished_pid = waitpid(-1, &status, WNOHANG)) > 0) {
        active_child_processes -= 1;
      }
    }
    
    // Если время вышло, а процессы еще остались - убиваем их
    if (active_child_processes > 0) {
      printf("Timeout reached (%d seconds). Sending SIGKILL to remaining child processes.\n", timeout);
      for (int i = 0; i < pnum; i++) {
        // Проверяем, жив ли еще процесс
        if (kill(child_pids[i], 0) == 0) {
          kill(child_pids[i], SIGKILL);
          printf("Sent SIGKILL to process %d\n", child_pids[i]);
        }
      }
      
      // Ждем завершения убитых процессов
      while (active_child_processes > 0) {
        wait(NULL);
        active_child_processes -= 1;
      }
    }
  } else {
    // Оригинальное поведение - просто ждем все процессы
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
      char filename[20];
      printf(filename, "minmax_%d.txt", i);
      FILE *file = fopen(filename, "r");
      if (file == NULL) {
          printf("Failed to open file\n");
          return 1;
      }
      fscanf(file, "%d %d", &min, &max);
      fclose(file);
      remove(filename);
  } else {
      // read from pipes
      read(pipe_fd[0], &min_max, sizeof(struct MinMax));
      min = min_max.min;
      max = min_max.max;
  }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
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
  fflush(NULL);
  return 0;
}