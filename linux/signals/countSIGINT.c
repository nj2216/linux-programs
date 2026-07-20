#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int count = 0;

void handler(int sig) {
	count++;
	printf("Caught SIGINT (%d/5)\n", count);
	if (count >= 5) {
		printf("Exiting now.\n");
		exit(0);
	}
}

int main() {
	signal(SIGINT, handler);
	while(1) pause();
	return 0;
}
