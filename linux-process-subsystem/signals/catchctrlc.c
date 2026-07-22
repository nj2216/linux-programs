#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void handler(int sig) {
	printf("Caught SIGNINT\n");
}

int main() {
	signal(SIGINT, handler);
	while(1){
		pause();
	}
	return 0;
}
