#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>


void exit_handler(void) {
    printf("[atexit] Завершение процесса с PID = %d\n", (int)getpid());
}


void sigint_handler(int sig) {
    printf("Получен сигнал SIGINT (номер %d) в процессе PID = %d\n", sig, (int)getpid());
}


void sigterm_handler(int sig, siginfo_t *info, void *context) {
    printf("Получен сигнал SIGTERM (номер %d) в процессе PID = %d\n", sig, (int)getpid());
}

int main() {
 
    if (atexit(exit_handler) != 0) {
        perror("atexit failed");
        exit(EXIT_FAILURE);
    }

  
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("signal(SIGINT) failed");
        exit(EXIT_FAILURE);
    }

 
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = sigterm_handler;
    sa.sa_flags = SA_SIGINFO;
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction(SIGTERM) failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
       
        printf("Дочерний процесс: PID = %d, PPID = %d\n", (int)getpid(), (int)getppid());
        sleep(2); 
        exit(42); 
    } else {
       
        printf("Родительский процесс: PID = %d, PPID = %d\n", (int)getpid(), (int)getppid());

        int status;
        pid_t waited_pid = wait(&status);

        if (WIFEXITED(status)) {
            printf("Дочерний процесс (PID = %d) завершился с кодом: %d\n",
                   (int)waited_pid, WEXITSTATUS(status));
        } else {
            printf("Дочерний процесс завершился необычно.\n");
        }
    }

    return 0;
}