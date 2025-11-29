#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

void demonstrate_zombie() {
    printf("=== Демонстрация зомби-процессов ===\n");
    
    pid_t pid = fork();
    
    if (pid == 0) {
        printf("Дочерний процесс: PID = %d, PPID = %d\n", getpid(), getppid());
        printf("Дочерний процесс завершает работу, но не убирается из таблицы процессов\n");
        exit(0);
    } else if (pid > 0) {
        printf("Родительский процесс: PID = %d, создал дочерний с PID = %d\n", getpid(), pid);
        printf("Родительский процесс не вызывает wait() для дочернего\n");

        sleep(2);
        
        printf("\nДочерний процесс стал зомби и тд\n");
        //printf("Выполните команду: ps aux | grep %d\n", pid);
        //printf("Или: ps -eo pid,ppid,state,comm | grep Z\n");
        printf("Нажмите Enter");
        getchar();
        
        int status;
        waitpid(pid, &status, 0);
        printf("Вызван waitpid(), зомби-процесс убран\n");
    } else {
        perror("Ошибка при создании процесса");
        exit(1);
    }
}

void multiple_zombies() {
    printf("\n=== Создание нескольких зомби-процессов ===\n");
    
    int num_zombies = 3;
    pid_t pids[3];
    
    for (int i = 0; i < num_zombies; i++) {
        pid_t pid = fork();
        
        if (pid == 0) {
            printf("Зомби %d: PID = %d завершает работу\n", i+1, getpid());
            exit(0);
        } else if (pid > 0) {
            pids[i] = pid;
            printf("Создан дочерний процесс %d с PID = %d\n", i+1, pid);
        } else {
            perror("Ошибка fork");
            exit(1);
        }
    }
    
    sleep(2);
    printf("\nСоздано %d зомби-процессов\n", num_zombies);
    printf("Нажмите Enter для очистки");
    getchar();
    
    for (int i = 0; i < num_zombies; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        printf("Зомби с PID = %d убран\n", pids[i]);
    }
}

void prevent_zombie_with_wait() {
    printf("\n=== Предотвращение зомби с помощью wait() ===\n");
    
    pid_t pid = fork();
    
    if (pid == 0) {
        printf("Дочерний процесс %d работает\n", getpid());
        sleep(1);
        printf("Дочерний процесс %d завершается\n", getpid());
        exit(0);
    } else if (pid > 0) {
        printf("Родительский процесс %d ждет завершения дочернего\n", getpid());
        int status;
        wait(&status);
        printf("Родительский процесс: дочерний процесс корректно завершен\n");
    } else {
        perror("Ошибка fork");
        exit(1);
    }
}

void prevent_zombie_with_signal() {
    printf("\n=== Предотвращение зомби с помощью сигнала SIGCHLD ===\n");
    
    signal(SIGCHLD, SIG_IGN);
    
    pid_t pid = fork();
    
    if (pid == 0) {
        printf("Дочерний процесс %d работает\n", getpid());
        sleep(1);
        printf("Дочерний процесс %d завершается\n", getpid());
        exit(0);
    } else if (pid > 0) {
        printf("Родительский процесс %d продолжает работу (SIGCHLD игнорируется)\n", getpid());
        printf("Дочерний процесс автоматически уберется ядром\n");
        sleep(2);
    } else {
        perror("Ошибка fork");
        exit(1);
    }
}

int main() {
    printf("Демонстрация зомби-процессов\n");
    printf("=====================================\n\n");
    
    int choice;
    
    while (1) {
        printf("\nВыберите демонстрацию:\n");
        printf("1 - Один зомби-процесс\n");
        printf("2 - Несколько зомби-процессов\n");
        printf("3 - Предотвращение зомби с wait()\n");
        printf("4 - Предотвращение зомби с SIGCHLD\n");
        printf("0 - Выход\n");
        printf("Ваш выбор: ");
        
        scanf("%d", &choice);
        getchar();
        
        switch (choice) {
            case 1:
                demonstrate_zombie();
                break;
            case 2:
                multiple_zombies();
                break;
            case 3:
                prevent_zombie_with_wait();
                break;
            case 4:
                prevent_zombie_with_signal();
                break;
            case 0:
                printf("Выход\n");
                return 0;
            default:
                printf("Выбор не выбор\n");
        }
    }
    
    return 0;
}