#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();

    if (pid == 0) {
        printf("Child (pid %d): parent is currently %d\n", getpid(), getppid());
        printf("Child: sleeping for 10 seconds...\n");
        sleep(10);

        printf("Child (pid %d): woke up. New parent pid is now %d\n",
               getpid(), getppid());

    } else {
        printf("Parent (pid %d): exiting immediately, orphaning child (pid %d).\n",
               getpid(), pid);
        _exit(0); 
    }

    return 0;
}