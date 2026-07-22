#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

void sigchld_handler(int sig) {
    int status;
    pid_t pid;

    // Reap ALL children that have exited (WNOHANG = don't block if none ready)
    // Loop because multiple children could have exited before this handler
    // even got a chance to run (signals aren't queued).
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // Only async-signal-safe calls allowed here (e.g. write()), but
        // for a learning exercise printf is fine for visibility.
        printf("Reaper: reaped child pid %d (status %d)\n", pid, status);
    }
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; // restart interrupted syscalls like sleep()

    sigaction(SIGCHLD, &sa, NULL);

    // Spawn several children with different lifespans
    for (int i = 0; i < 5; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            // ---- CHILD ----
            int work_time = i + 1;
            printf("Child %d (pid %d): working for %d seconds.\n",
                   i, getpid(), work_time);
            sleep(work_time);
            printf("Child %d (pid %d): exiting.\n", i, getpid());
            _exit(0);
        }
        // parent continues the loop to spawn the next child
    }

    // Parent does its own work, no need to call wait() manually —
    // SIGCHLD handler reaps children automatically as they finish
    printf("Parent (pid %d): doing other work for 8 seconds...\n", getpid());
    sleep(8);

    printf("Parent: done. All children should have been auto-reaped.\n");
    printf("Parent: check 'ps' now — there should be no zombies left.\n");

    return 0;
}