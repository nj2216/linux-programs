#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int main() {
    srand(time(NULL));

    for(int i = 0; i < 3; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            int sleep_time = (rand() %4) +1;
            printf("Child: %d, PID: %d, PPID: %d, Sleep for: %d\n", i+1, getpid(), getppid(), sleep_time);
            sleep(sleep_time);
            exit(10+i);
        } else if (pid < 0) {
            perror("Failed");
            exit(1);
        }
    }

    int status;
    pid_t dead_pid;
    for(int i = 0; i < 3; i++) {
        dead_pid = wait(&status);
        if((status & 0x7F) == 0) {
            int exit_code = (status >> 8) & 0xFF;
            printf("Parent got PID = %d, Status Code = %d\n", dead_pid, exit_code);
        }
    }
    return 0;
}