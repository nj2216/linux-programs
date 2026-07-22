#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid == 0) {
        printf("Child (pid %d): exiting immediately.\n", getpid());
        _exit(0);
    } else {
        printf("Parent (pid %d): child pid is %d\n", getpid(), pid);
        printf("Parent sleeping for 30 seconds WITHOUT calling wait().\n");
        printf("Parent: run this in another terminal to observe the zombie:\n\n");
        printf("    ps -e1 | grep defunct\n");
        printf("    ps -o pid,ppid,stat,cmd -p %d\n\n", pid);

        sleep(15);

        printf("Parent: now reaping the zombie with wait().\n");
        wait(NULL);

        printf("Parent: zombie reaped, exiting\n");
    }
    return 0;
}