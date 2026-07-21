#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

#define NUM_WORKERS 4

volatile sig_atomic_t completed_count = 0;

void handler(int sig) {
    completed_count++;
}

int main() {
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    pid_t parent_pid = getpid();
    pid_t children[NUM_WORKERS];

    for (int i = 0; i < NUM_WORKERS; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            int work_time = (i + 1) * 2;
            printf("Worker %d (pid %d): workingfor %d seconds...\n", i, getpid(), work_time);
            
            sleep(work_time);

            printf("Worker %d (pid %d): finished, notifying parent.\n", i, getpid());
            kill(parent_pid, SIGUSR1);
            _exit(0);
        } else if (pid > 0) {
            children[i] = pid;
        } else {
            perror("fork failed");
            exit(1);
        }
    }

    printf("Parent: waiting for %d workers to complete...\n", NUM_WORKERS);
    
    while (completed_count < NUM_WORKERS) {
        pause();
        printf("Parent: %d/%d workers completed so far.\n",completed_count, NUM_WORKERS);
    }

    printf("Parent: all workers finished!\n");
    for (int i = 0; i < NUM_WORKERS; i++) {
        waitpid(children[i], NULL, 0);
        return 0;
    }
}