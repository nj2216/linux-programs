#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

void safe_handler(int sig) {
    const char msg[] = "Caught SIGINT (safe handler)\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = safe_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);

    while(1) pause();

    return 0;
}