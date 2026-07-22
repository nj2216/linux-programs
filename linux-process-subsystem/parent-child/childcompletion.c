#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

volatile sig_atomic_t child_done = 0;

void handler(int sig) {
	child_done = 1;
}

int main() {
	struct sigaction sa;
	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGUSR1, &sa, NULL);

	pid_t pid = fork();

	if(pid == 0) {
		pid_t parent_pid = getppid();

		printf("Child: starting work...\n");
		sleep(4);
		printf("Child: work finished...\n");

		kill(parent_pid, SIGUSR1);
		_exit(0);
	} else {
		printf("Parent: waiting for child to finish (no polling)...\n");

		while(!child_done) {
			pause();
		}
		printf("Parent: notified! Child has completed its work.\n");
		wait(NULL);
		printf("Parent: done.\n");
	}
	return 0;
}
