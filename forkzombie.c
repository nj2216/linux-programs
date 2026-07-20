#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();

    if (pid == 0) {
        printf("Child exiting immediately...\n");
        exit(0); 
    } else {
        printf("Parent sleeping. Run 'ps -elf | grep defunct' to see the zombie child.\n");
        sleep(20); // Parent stays alive without calling wait()
    }
    return 0;
}

