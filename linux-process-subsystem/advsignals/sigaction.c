#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

/* ---------- Exercise 8 equivalent ---------- */
void handler_sigint(int sig) {
    printf("Caught SIGINT\n");
}

void ex8_sigaction() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler_sigint;
    sigemptyset(&sa.sa_mask);   // no extra signals blocked during handler
    sa.sa_flags = 0;            // no SA_RESTART, no special flags

    sigaction(SIGINT, &sa, NULL);

    while (1) pause();
}

/* ---------- Exercise 9 equivalent ---------- */
void handler_alarm(int sig) {
    printf("Time's up!\n");
}

void ex9_sigaction() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler_alarm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGALRM, &sa, NULL);
    alarm(5);
    pause();
}

/* ---------- Exercise 10 equivalent ---------- */
int count = 0;

void handler_count(int sig) {
    count++;
    printf("Caught SIGINT (%d/5)\n", count);
    if (count >= 5) {
        printf("Exiting now.\n");
        exit(0);
    }
}

void ex10_sigaction() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler_count;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    while (1) pause();
}

/* ---------- Exercise 11 equivalent ---------- */
void ex11_sigaction() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigaction(SIGINT, &sa, NULL);

    printf("PID: %d — Ctrl+C is ignored. Use kill -SIGTERM %d\n", getpid(), getpid());
    while (1) pause();
}

/* ---------- Exercise 12 equivalent ---------- */
void handler_usr1(int sig) {
    printf("Received SIGUSR1\n");
}

void ex12_sigaction() {
    pid_t pid = fork();

    if (pid == 0) {
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = handler_usr1;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        sigaction(SIGUSR1, &sa, NULL);
        pause();
    } else {
        sleep(3);
        kill(pid, SIGUSR1);
        wait(NULL);
    }
}

int main() {
    // Call whichever exercise you want to test, e.g.:
    ex8_sigaction();
    return 0;
}