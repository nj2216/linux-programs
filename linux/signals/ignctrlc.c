#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int main() {
	signal(SIGINT, SIG_IGN);

	printf("PID: %d - try Ctrl +C, it won't work.\n", getpid());
	printf("Use: kill -SIGTERM %d (or kill -9) to stop this.\n", getpid());

	while(1) pause();

	return 0;
}
