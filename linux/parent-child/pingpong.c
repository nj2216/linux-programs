#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

volatile sig_atomic_t got_signal = 0;

void handler(int sig) {
	got_signal = sig;
}


int main() {
	struct sigaction sa;
	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);

	pid_t pid = fork();

	if(pid == 0) {
		pid_t parent_pid = getppid();
		for(int i = 0; i < 10; i++) {
			got_signal = 0;
			while (got_signal != SIGUSR1) {
				pause();
			}
			printf("Child: received PING (SIGUSR1) #%d\n", i + 1);

			kill(parent_pid, SIGUSR2);
			printf("Child: sent PONG (SIGUSR2) #%d\n", i + 1);
		}
		printf("Child: done, exiting.\n");
	} else {
		sleep(1);
		for(int i = 0; i < 10; i++) {
			kill(pid, SIGUSR1);
			printf("Parent: sent PING (SIGUSR1) #%d\n", i + 1);

			got_signal = 0;
			while (got_signal != SIGUSR2) {
				pause();
			}
			printf("Parent: received PONG (SIGUSR2) #%d\n", i + 1);
		}
		wait(NULL);
		printf("Parent: done.\n");
	}
	return 0;
}
