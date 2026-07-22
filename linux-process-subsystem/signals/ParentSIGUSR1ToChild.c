#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

void handler(int sig) {
    printf("Received SIGUSR1\n");
}

int main() {
    pid_t pid = fork();

    if (pid == 0) {
        signal(SIGUSR1, handler);
        pause();
    } else {
        sleep(3);
        kill(pid, SIGUSR1);
        wait(NULL);
    }

    return 0;
}