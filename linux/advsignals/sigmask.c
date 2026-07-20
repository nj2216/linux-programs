#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int main() {
    sigset_t block_set, old_set;

    sigemptyset(&block_set);
    sigaddset(&block_set, SIGINT);

    printf("Blocking SIGINT for 10 secs.\n");

    sigprocmask(SIG_BLOCK, &block_set, &old_set);

    sleep(10);

    printf("Unblocking now\n");
    sigprocmask(SIG_SETMASK, &old_set, NULL);

    printf("If any SIGINT was pending, it should now be delivered immediately.\n");

    // Give a moment to observe default action (usually terminates the program)
    sleep(2);
    printf("Still alive — no handler was installed, so default action must not have fired,\n");
    printf("or this line ran before delivery. Try adding a handler to see it caught instead.\n");

    return 0;
}