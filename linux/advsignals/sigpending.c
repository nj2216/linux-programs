#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

void dummy_handler(int sig) {
    //does nothing
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = dummy_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    sigset_t block_set, pending_set;

    sigemptyset(&block_set);
    sigaddset(&block_set, SIGINT);
    sigprocmask(SIG_BLOCK, &block_set, NULL);

    printf("SIGINT is blocked...\n");

    sleep(5);

    sigpending(&pending_set);

    if (sigismember(&pending_set, SIGINT)) {
        printf("SIGINT is pending\n");
    } else {
        printf("No SIGINT pending\n");
    }

    printf("Unblocking now - pending signals will be passed..\n");
    sigprocmask(SIG_UNBLOCK, &block_set, NULL);

    sleep(1);
    printf("Done\n");
    return 0;
}